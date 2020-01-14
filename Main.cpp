#include <SFML/Graphics.hpp>
#include <glm/glm.hpp>
#include <glew.h>
#include <functional>

//strange things happening
//-Better performance on frames where you make new particles vs re-use them
//-every (emits per second) strange burst effect. only really noticable with color over time

int main(int argc, char** argv)
{
    //for dt later
    sf::Clock deltaClock;
    //open window
    sf::Window* window = new sf::Window(sf::VideoMode::getDesktopMode(), "ParticleTest", sf::Style::Fullscreen, sf::ContextSettings());
    window->setActive(true);
    //setup glew
    GLenum value_enum = glewInit();

    //basic opengl intitialization
    glClearColor(0, 0, 0, 1);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // load shaders
    const char* fragment_shader_text =
        "#version 130\n\
        uniform sampler2D usamp;\
        in float vtrans;\
        in vec4 vtint;\
        in vec2 vtexcoord;\
        out vec4 frag_color;\
        void main(void) {\
            frag_color = texture(usamp, vtexcoord);\
            frag_color.a *= vtrans;\
            frag_color.xyz = frag_color.xyz * (1 - vtint.a) + vtint.xyz * (vtint.a);\
        }";

    GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fshader, 1, &fragment_shader_text, 0);
    glCompileShader(fshader);
    GLint test;
    glGetShaderiv(fshader, GL_COMPILE_STATUS, &test);
    if (!test)
    {
        char buffer[1024];
        glGetShaderInfoLog(fshader, 1024, 0, buffer);
        int bad = 0;
    }

    const char* vertex_shader_text =
        "#version 130\n\
         in vec4 vertData;\
         in vec3 imageData;\
         \n\
         in float t;\
         in vec2 pos0; in vec4 post;\
         in float rot0; in vec2 rott;\
         in vec2 scale0; in vec4 scalet;\
         in float trans0; in vec2 transt;\
         in vec4 tint0; in vec4 tintt;\
         in float frameDur;\
         \n\
         out vec2 vtexcoord;\
         out float vtrans;\
         out vec4 vtint;\
         mat3 modelMatrix(vec2 pos, vec2 size, float rot){\
             mat3 posMat = mat3(1, 0, 0, 0, 1, 0, pos.x, pos.y, 1);\
             mat3 sizeMat = mat3(size.x, 0, 0, 0, size.y, 0, 0, 0, 1);\
             mat3 rotMat = mat3(cos(radians(rot)), sin(radians(rot)), 0, -sin(radians(rot)), cos(radians(rot)), 0, 0, 0, 1);\
             return posMat * rotMat * sizeMat;\
         }\
         void main() {\
             vec2 pos = pos0 + post.xy * t + 0.5 * post.zw * t * t;\
             float rot = rot0 + rott.x * t + 0.5 * rott.y * t * t;\
             vec2 scale = scale0 + scalet.xy * t + 0.5 * scalet.zw * t * t;\
             vec3 adjustedPos = modelMatrix(pos, scale, rot) * vec3(vertData.xy, 1);\
             gl_Position = vec4(adjustedPos.xy, 0, 1);\
             \n\
             int numRow = int(imageData.x); int numCol = int(imageData.y); int numFrame = int(imageData.z);\
             int frame = int(mod(int(t / max(0.0000001, frameDur)), numFrame));\
             vec2 texCoord = vec2((vertData.z + int(mod(frame, numRow))) / numCol, (vertData.w + int(frame / numCol)) / numRow);\
             \n\
             vtexcoord = texCoord;\
             vtrans = trans0 + transt.x * t + 0.5 * transt.y * t * t; \
             vtint = tint0 + tintt * t;\
         }";

    GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vshader, 1, &vertex_shader_text, 0);
    glCompileShader(vshader);
    glGetShaderiv(vshader, GL_COMPILE_STATUS, &test);
    if (!test)
    {
        char buffer[1024];
        glGetShaderInfoLog(vshader, 1024, 0, buffer);
        int bad = 0;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, fshader); glAttachShader(program, vshader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &test);
    if (!test)
    {
        char buffer[1024];
        glGetProgramInfoLog(program, 1024, 0, buffer);
        int bad = 0;
    }
    glDeleteShader(fshader); glDeleteShader(vshader);

    //link shader variables
    glUseProgram(program);
    GLuint aVertData = glGetAttribLocation(program, "vertData");
    glEnableVertexAttribArray(aVertData);

    GLuint aImageData = glGetAttribLocation(program, "imageData");
    glEnableVertexAttribArray(aImageData);

    GLuint aT = glGetAttribLocation(program, "t");
    glEnableVertexAttribArray(aT);

    GLuint aPos0 = glGetAttribLocation(program, "pos0");
    glEnableVertexAttribArray(aPos0);
    GLuint aPosT = glGetAttribLocation(program, "post");
    glEnableVertexAttribArray(aPosT);

    GLuint aRot0 = glGetAttribLocation(program, "rot0");
    glEnableVertexAttribArray(aRot0);
    GLuint aRotT = glGetAttribLocation(program, "rott");
    glEnableVertexAttribArray(aRotT);

    GLuint aScale0 = glGetAttribLocation(program, "scale0");
    glEnableVertexAttribArray(aScale0);
    GLuint aScaleT = glGetAttribLocation(program, "scalet");
    glEnableVertexAttribArray(aScaleT);

    GLuint aTrans0 = glGetAttribLocation(program, "trans0");
    glEnableVertexAttribArray(aTrans0);
    GLuint aTransT = glGetAttribLocation(program, "transt");
    glEnableVertexAttribArray(aTransT);

    GLuint aTint0 = glGetAttribLocation(program, "tint0");
    glEnableVertexAttribArray(aTint0);
    GLuint aTintT = glGetAttribLocation(program, "tintt");
    glEnableVertexAttribArray(aTintT);

    GLuint aFrameDur = glGetAttribLocation(program, "frameDur");
    glEnableVertexAttribArray(aFrameDur);

    //early knowns for buffers
    unsigned maxParticles = 100000;
    int particleDataSize = 27;
    std::vector<float> vertices = {
        0.5f, 0.5f, 1.f, 0.f,
        -0.5f, 0.5f, 0.f, 0.f,
        -0.5f, -0.5f, 0.f, 1.f,
        0.5f, -0.5f, 1.f, 1.f
    };
    std::vector<unsigned> indices = {
        0, 1, 2, 2, 3, 0
    };

    std::vector<float> imageData = {
       3, 3, 8 //rows, columns, frames
    };

    //tell the variables how to advance
    glVertexAttribDivisor(aVertData, 0);
    glVertexAttribDivisor(aImageData, maxParticles);
    glVertexAttribDivisor(aT, 1);

    glVertexAttribDivisor(aPos0, 1);
    glVertexAttribDivisor(aPosT, 1);
    glVertexAttribDivisor(aRot0, 1);
    glVertexAttribDivisor(aRotT, 1);
    glVertexAttribDivisor(aScale0, 1);
    glVertexAttribDivisor(aScaleT, 1);
    glVertexAttribDivisor(aTrans0, 1);
    glVertexAttribDivisor(aTransT, 1);
    glVertexAttribDivisor(aTint0, 1);
    glVertexAttribDivisor(aTintT, 1);
    glVertexAttribDivisor(aFrameDur, 1);


    //initialize the buffers and setup pointers for variables
    GLuint vertices_handle, indices_handle, imageData_handle, time_handle, data_handle;
    glGenBuffers(1, &vertices_handle);
    glGenBuffers(1, &indices_handle);
    glGenBuffers(1, &imageData_handle);
    glGenBuffers(1, &time_handle);
    glGenBuffers(1, &data_handle);

    glBindBuffer(GL_ARRAY_BUFFER, vertices_handle);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(aVertData, 4, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_handle);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned), indices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, imageData_handle);
    glBufferData(GL_ARRAY_BUFFER, imageData.size() * sizeof(float), imageData.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(aImageData, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, time_handle);
    glBufferData(GL_ARRAY_BUFFER, maxParticles * sizeof(float), NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(aT, 1, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, data_handle);
    glBufferData(GL_ARRAY_BUFFER, maxParticles * sizeof(float) * 27, NULL, GL_STREAM_DRAW);

    glVertexAttribPointer(aPos0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * particleDataSize, (GLvoid*)(sizeof(GL_FLOAT) * 0));
    glVertexAttribPointer(aPosT, 4, GL_FLOAT, GL_FALSE, sizeof(float) * particleDataSize, (GLvoid*)(sizeof(GL_FLOAT) * 2));

    glVertexAttribPointer(aRot0, 1, GL_FLOAT, GL_FALSE, sizeof(float) * particleDataSize, (GLvoid*)(sizeof(GL_FLOAT) * 6));
    glVertexAttribPointer(aRotT, 2, GL_FLOAT, GL_FALSE, sizeof(float) * particleDataSize, (GLvoid*)(sizeof(GL_FLOAT) * 7));

    glVertexAttribPointer(aScale0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * particleDataSize, (GLvoid*)(sizeof(GL_FLOAT) * 9));
    glVertexAttribPointer(aScaleT, 4, GL_FLOAT, GL_FALSE, sizeof(float) * particleDataSize, (GLvoid*)(sizeof(GL_FLOAT) * 11));

    glVertexAttribPointer(aTrans0, 1, GL_FLOAT, GL_FALSE, sizeof(float) * particleDataSize, (GLvoid*)(sizeof(GL_FLOAT) * 15));
    glVertexAttribPointer(aTransT, 2, GL_FLOAT, GL_FALSE, sizeof(float) * particleDataSize, (GLvoid*)(sizeof(GL_FLOAT) * 16));

    glVertexAttribPointer(aTint0, 4, GL_FLOAT, GL_FALSE, sizeof(float) * particleDataSize, (GLvoid*)(sizeof(GL_FLOAT) * 18));
    glVertexAttribPointer(aTintT, 4, GL_FLOAT, GL_FALSE, sizeof(float) * particleDataSize, (GLvoid*)(sizeof(GL_FLOAT) * 22));

    glVertexAttribPointer(aFrameDur, 1, GL_FLOAT, GL_FALSE, sizeof(float) * particleDataSize, (GLvoid*)(sizeof(GL_FLOAT) * 26));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    //texture
    char pathName[FILENAME_MAX] = "";
    sprintf_s(pathName, _countof(pathName), "Assets/MonkeyWalk.png");
    sf::Image loadedImage;
    loadedImage.loadFromFile(pathName);

    GLuint texture_handle;
    glGenTextures(1, &texture_handle);
    glBindTexture(GL_TEXTURE_2D, texture_handle);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, loadedImage.getSize().x, loadedImage.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, loadedImage.getPixelsPtr());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);

    //emitter variables
    std::vector<float> timeData, startData;
    std::vector<bool>  aliveData;

    //emit settings
    float particleLifeTime = 3.f;
    float particleDeathTime = 5.f;

    float particlesPerSecond = 10000;
    unsigned particlesPerBurst = 1;
    float emitTimer = 0;

    bool emitterStatic = true;

    //particle settings
    glm::vec2 posStart(0, 0);
    glm::vec2 posOverTime(0, 0);
    glm::vec2 posOverTime2(-0.5, 0);

    float rotStart = 50.f;
    float rotOverTime = 20.f;
    float rotOverTime2 = 0;

    glm::vec2 scaleStart(0.1, 0.1);
    glm::vec2 scaleOverTime(0, 0);
    glm::vec2 scaleOverTime2(0, 0);

    float transStart = 1.f;
    float transOverTime = 0.f;
    float transOverTime2 = 0.f;

    glm::vec4 tintStart(0, 0.7, 0.5, 0.7);
    glm::vec4 tintOverTime(1, 0, 0, 0.0);

    float frameDuration = 10;

    //start loop
    while (true)
    {
        //get dt
        float dt = deltaClock.restart().asSeconds();
        printf("Framerate: %f\n", 1 / dt);

        //calc number of particles to emit
        unsigned particlesToEmit = 0;
        while (emitTimer <= 0)
        {
            particlesToEmit += particlesPerBurst;
            emitTimer += 1 / particlesPerSecond;
        }
        emitTimer -= dt;

        bool dataChanged = false; //any time particle data is changed, need to alert gpu of change

        //update times for current particles
        int index = -1;
        std::vector<unsigned> toRemove;
        std::for_each(timeData.begin(), timeData.end(), [&](float& time)
        {
            time += dt;
            ++index;

            if (time < particleLifeTime)
                return;

            if (emitterStatic)
            {
                time = 0;
                return;
            }

            //re-use now
            if (particlesToEmit > 0)
            {
                --particlesToEmit;
                time = 0;

                dataChanged = true;
                aliveData[index] = true;
                int dataIndex = index * particleDataSize;
                startData[dataIndex + 0] = posStart.x;
                startData[dataIndex + 1] = posStart.y;
                glm::vec2 velo = glm::vec2((((float)rand() / (RAND_MAX)) * 0.5 - 0.25), ((float)rand() / (RAND_MAX)) * 0.5 - 0.25);
                //startData[dataIndex +  2] = posOverTime.x;
                startData[dataIndex + 2] = velo.x;
                //startData[dataIndex +  3] = posOverTime.y;
                startData[dataIndex + 3] = velo.y;
                startData[dataIndex + 4] = posOverTime2.x;
                startData[dataIndex + 5] = posOverTime2.y;
                startData[dataIndex + 6] = rotStart;
                startData[dataIndex + 7] = rotOverTime;
                startData[dataIndex + 8] = rotOverTime2;
                startData[dataIndex + 9] = scaleStart.x;
                startData[dataIndex + 10] = scaleStart.y;
                startData[dataIndex + 11] = scaleOverTime.x;
                startData[dataIndex + 12] = scaleOverTime.y;
                startData[dataIndex + 13] = scaleOverTime2.x;
                startData[dataIndex + 14] = scaleOverTime2.y;
                startData[dataIndex + 15] = transStart;
                startData[dataIndex + 16] = transOverTime;
                startData[dataIndex + 17] = transOverTime2;
                startData[dataIndex + 18] = tintStart.r;
                startData[dataIndex + 19] = tintStart.g;
                startData[dataIndex + 20] = tintStart.b;
                startData[dataIndex + 21] = tintStart.a;
                startData[dataIndex + 22] = tintOverTime.r;
                startData[dataIndex + 23] = tintOverTime.g;
                startData[dataIndex + 24] = tintOverTime.b;
                startData[dataIndex + 25] = tintOverTime.a;
                startData[dataIndex + 26] = frameDuration;
                return;
            }

            //particle has beed dead for too long, remove from data
            if (time > particleLifeTime + particleDeathTime)
            {
                toRemove.push_back(index);
                dataChanged = true;
                return;
            }

            //particle needs to be made dead
            else if (time > particleLifeTime && aliveData[index])
            {
                //set all data to 0
                std::fill(startData.begin() + index * particleDataSize, startData.begin() + (index + 1) * particleDataSize, 0);
                aliveData[index] = false;
                dataChanged = true;
                return;
            }
        });

        //remove data if need be
        if (toRemove.size())
        {
            //remove from time data
            unsigned removeIndex = 0; index = 0;
            std::remove_if(timeData.begin(), timeData.end(), [&](float)
            {
                if (removeIndex >= toRemove.size())
                    return false;
                if (index++ == toRemove[removeIndex++])
                    return true;
                return false;
            });
            timeData.erase(timeData.end() - toRemove.size(), timeData.end());

            //remove from alive data
            removeIndex = 0; index = 0;
            std::remove_if(aliveData.begin(), aliveData.end(), [&](float)
            {
                if (removeIndex >= toRemove.size())
                    return false;
                if (index++ == toRemove[removeIndex++])
                    return true;
                return false;
            });
            aliveData.erase(aliveData.end() - toRemove.size(), aliveData.end());

            //remove from start data
            removeIndex = 0; index = 0;
            std::remove_if(startData.begin(), startData.end(), [&](float)
            {
                if (removeIndex >= toRemove.size())
                    return false;
                if (index++ / particleDataSize == toRemove[removeIndex++])
                    return true;
                return false;
            });
            startData.erase(startData.end() - toRemove.size() * particleDataSize, startData.end());
        }

        //create new particles (should never happen in same loop as deleting because of recycling)
        for (unsigned i = 0; i < particlesToEmit && timeData.size() < maxParticles; i++)
        {
            dataChanged = true;

            timeData.push_back(0);
            aliveData.push_back(true);
            startData.push_back(posStart.x);
            startData.push_back(posStart.y);
            glm::vec2 velo = glm::vec2((((float)rand() / (RAND_MAX)) * 0.5 - 0.25), ((float)rand() / (RAND_MAX)) * 0.5 - 0.25);
            //startData.push_back(posOverTime.x);
            startData.push_back(velo.x);
            //startData.push_back(posOverTime.y);
            startData.push_back(velo.y);
            startData.push_back(posOverTime2.x);
            startData.push_back(posOverTime2.y);
            startData.push_back(rotStart);
            startData.push_back(rotOverTime);
            startData.push_back(rotOverTime2);
            startData.push_back(scaleStart.x);
            startData.push_back(scaleStart.y);
            startData.push_back(scaleOverTime.x);
            startData.push_back(scaleOverTime.y);
            startData.push_back(scaleOverTime2.x);
            startData.push_back(scaleOverTime2.y);
            startData.push_back(transStart);
            startData.push_back(transOverTime);
            startData.push_back(transOverTime2);
            startData.push_back(tintStart.r);
            startData.push_back(tintStart.g);
            startData.push_back(tintStart.b);
            startData.push_back(tintStart.a);
            startData.push_back(tintOverTime.r);
            startData.push_back(tintOverTime.g);
            startData.push_back(tintOverTime.b);
            startData.push_back(tintOverTime.a);
            startData.push_back(frameDuration);
        }

        printf("%f size: %i particlesToEmit: %i\n", 1 / dt, timeData.size(), particlesToEmit);

        //if data was changed, either new particles were emitted, particle died, or was deleted
        if (dataChanged)
        {
            //push the new data
            glBindBuffer(GL_ARRAY_BUFFER, data_handle);
            glBufferData(GL_ARRAY_BUFFER, maxParticles * sizeof(float) * particleDataSize, NULL, GL_STREAM_DRAW);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * startData.size(), startData.data());
        }

        //push the new times for the particles
        glBindBuffer(GL_ARRAY_BUFFER, time_handle);
        glBufferData(GL_ARRAY_BUFFER, maxParticles * sizeof(float), NULL, GL_STREAM_DRAW); //buffer orphaning
        glBufferSubData(GL_ARRAY_BUFFER, 0, timeData.size() * sizeof(float), timeData.data());

        //draw instances of particles
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_handle);
        glBindTexture(GL_TEXTURE_2D, texture_handle);
        glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, timeData.size());

        window->display();
        glClear(GL_COLOR_BUFFER_BIT);

        //things to start with that are the same until recycled
        //position_0 + position/time + position/time^2 (vec2) * 3  = 6
        //rotation_0 + rotation/time + rotation/time^2 (float) * 3 = 3
        //scale_0 + scale/time + scale/time^2          (vec2) * 3  = 6
        //transp_0 + transp/time + transp/time^2       (float) * 3 = 3
        //tint_0 + tint/time                           (vec4) * 2  = 8
        //frameduration                                (float)     = 1
        //                                                          27 floats per particle
        //
        //could be all the same for every particle, any number of them could be different
        //      maybe change based on how many get read in per element?
    }

    delete window;
    glDeleteProgram(program);
    return EXIT_SUCCESS;
}

/*
int main(int argc, char** argv)
{
    //for dt later
    sf::Clock deltaClock;
    //open window
    sf::Window* window = new sf::Window(sf::VideoMode::getDesktopMode(), "test", sf::Style::Fullscreen, sf::ContextSettings());
    window->setActive(true);
    //setup glew
    GLenum value_enum = glewInit();

    //basic opengl intitialization
    glClearColor(0, 0, 0, 1);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // load shaders
    const char* fragment_shader_text =
        "#version 130\n\
        uniform sampler2D usamp;\
        in float vtrans;\
        in vec4 vtint;\
        in vec2 vtexcoord;\
        out vec4 frag_color;\
        void main(void) {\
            frag_color = texture(usamp, vtexcoord);\
            frag_color.a *= vtrans;\
            frag_color.xyz = frag_color.xyz * (1 - vtint.a) + vtint.xyz * (vtint.a);\
        }";

    GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fshader, 1, &fragment_shader_text, 0);
    glCompileShader(fshader);
    GLint test;
    glGetShaderiv(fshader, GL_COMPILE_STATUS, &test);
    if (!test)
    {
        char buffer[1024];
        glGetShaderInfoLog(fshader, 1024, 0, buffer);
        int bad = 0;
    }

    const char* vertex_shader_text =
        "#version 130\n\
         in vec4 vertData;\
         in vec3 imageData;\
         \n\
         in float t;\
         in vec2 pos0; in vec4 post;\
         in float rot0; in vec2 rott;\
         in vec2 scale0; in vec4 scalet;\
         in float trans0; in vec2 transt;\
         in vec4 tint0; in vec4 tintt;\
         in float frameDur;\
         \n\
         out vec2 vtexcoord;\
         out float vtrans;\
         out vec4 vtint;\
         mat3 modelMatrix(vec2 pos, vec2 size, float rot){\
             mat3 posMat = mat3(1, 0, 0, 0, 1, 0, pos.x, pos.y, 1);\
             mat3 sizeMat = mat3(size.x, 0, 0, 0, size.y, 0, 0, 0, 1);\
             mat3 rotMat = mat3(cos(radians(rot)), sin(radians(rot)), 0, -sin(radians(rot)), cos(radians(rot)), 0, 0, 0, 1);\
             return posMat * rotMat * sizeMat;\
         }\
         void main() {\
             vec2 pos = pos0 + post.xy * t + 0.5 * post.zw * t * t;\
             float rot = rot0 + rott.x * t + 0.5 * rott.y * t * t;\
             vec2 scale = scale0 + scalet.xy * t + 0.5 * scalet.zw * t * t;\
             vec3 adjustedPos = modelMatrix(pos, scale, rot) * vec3(vertData.xy, 1);\
             gl_Position = vec4(adjustedPos.xy, 0, 1);\
             \n\
             int numRow = int(imageData.x); int numCol = int(imageData.y); int numFrame = int(imageData.z);\
             int frame = int(mod(int(t / max(0.0000001, frameDur)), numFrame));\
             vec2 texCoord = vec2((vertData.z + int(mod(frame, numRow))) / numCol, (vertData.w + int(frame / numCol)) / numRow);\
             \n\
             vtexcoord = texCoord;\
             vtrans = trans0 + transt.x * t + 0.5 * transt.y * t * t; \
             vtint = tint0 + tintt * t;\
         }";

    GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vshader, 1, &vertex_shader_text, 0);
    glCompileShader(vshader);
    glGetShaderiv(vshader, GL_COMPILE_STATUS, &test);
    if (!test)
    {
        char buffer[1024];
        glGetShaderInfoLog(vshader, 1024, 0, buffer);
        int bad = 0;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, fshader); glAttachShader(program, vshader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &test);
    if (!test)
    {
        char buffer[1024];
        glGetProgramInfoLog(program, 1024, 0, buffer);
        int bad = 0;
    }
    glDeleteShader(fshader); glDeleteShader(vshader);

    //link shader variables
    glUseProgram(program);
    GLuint aVertData = glGetAttribLocation(program, "vertData");
    glEnableVertexAttribArray(aVertData);

    GLuint aImageData = glGetAttribLocation(program, "imageData");
    glEnableVertexAttribArray(aImageData);

    GLuint aT = glGetAttribLocation(program, "t");
    glEnableVertexAttribArray(aT);

    GLuint aPos0 = glGetAttribLocation(program, "pos0");
    glEnableVertexAttribArray(aPos0);
    GLuint aPosT = glGetAttribLocation(program, "post");
    glEnableVertexAttribArray(aPosT);

    GLuint aRot0 = glGetAttribLocation(program, "rot0");
    glEnableVertexAttribArray(aRot0);
    GLuint aRotT = glGetAttribLocation(program, "rott");
    glEnableVertexAttribArray(aRotT);

    GLuint aScale0 = glGetAttribLocation(program, "scale0");
    glEnableVertexAttribArray(aScale0);
    GLuint aScaleT = glGetAttribLocation(program, "scalet");
    glEnableVertexAttribArray(aScaleT);

    GLuint aTrans0 = glGetAttribLocation(program, "trans0");
    glEnableVertexAttribArray(aTrans0);
    GLuint aTransT = glGetAttribLocation(program, "transt");
    glEnableVertexAttribArray(aTransT);

    GLuint aTint0 = glGetAttribLocation(program, "tint0");
    glEnableVertexAttribArray(aTint0);
    GLuint aTintT = glGetAttribLocation(program, "tintt");
    glEnableVertexAttribArray(aTintT);

    GLuint aFrameDur = glGetAttribLocation(program, "frameDur");
    glEnableVertexAttribArray(aFrameDur);

    //early knowns for buffers
    unsigned maxParticles = 100000;
    int particleDataSize = 27;
    std::vector<float> vertices = {
        0.5f, 0.5f, 1.f, 0.f,
        -0.5f, 0.5f, 0.f, 0.f,
        -0.5f, -0.5f, 0.f, 1.f,
        0.5f, -0.5f, 1.f, 1.f
    };
    std::vector<unsigned> indices = {
        0, 1, 2, 2, 3, 0
    };

    std::vector<float> imageData = {
       3, 3, 8 //rows, columns, frames
    };

    //tell the variables how to advance
    glVertexAttribDivisor(aVertData, 0);
    glVertexAttribDivisor(aImageData, maxParticles);
    glVertexAttribDivisor(aT, 1);

    glVertexAttribDivisor(aPos0, 1);
    glVertexAttribDivisor(aPosT, 1);
    glVertexAttribDivisor(aRot0, 1);
    glVertexAttribDivisor(aRotT, 1);
    glVertexAttribDivisor(aScale0, 1);
    glVertexAttribDivisor(aScaleT, 1);
    glVertexAttribDivisor(aTrans0, 1);
    glVertexAttribDivisor(aTransT, 1);
    glVertexAttribDivisor(aTint0, 1);
    glVertexAttribDivisor(aTintT, 1);
    glVertexAttribDivisor(aFrameDur, 1);


    //initialize the buffers and setup pointers for variables
    GLuint vertices_handle, indices_handle, imageData_handle, time_handle, data_handle;
    glGenBuffers(1, &vertices_handle);
    glGenBuffers(1, &indices_handle);
    glGenBuffers(1, &imageData_handle);
    glGenBuffers(1, &time_handle);
    glGenBuffers(1, &data_handle);

    glBindBuffer(GL_ARRAY_BUFFER, vertices_handle);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(aVertData, 4, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_handle);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned), indices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, imageData_handle);
    glBufferData(GL_ARRAY_BUFFER, imageData.size() * sizeof(float), imageData.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(aImageData, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, time_handle);
    glBufferData(GL_ARRAY_BUFFER, maxParticles * sizeof(float), NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(aT, 1, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, data_handle);
    glBufferData(GL_ARRAY_BUFFER, maxParticles * sizeof(float) * 27, NULL, GL_STREAM_DRAW);

    glVertexAttribPointer(aPos0, 2, GL_FLOAT, GL_FALSE,     sizeof(float) * particleDataSize, (GLvoid*)(sizeof(GL_FLOAT) * 0));
    glVertexAttribPointer(aPosT, 4, GL_FLOAT, GL_FALSE,     sizeof(float) * particleDataSize, (GLvoid*)(sizeof(GL_FLOAT) * 2));

    glVertexAttribPointer(aRot0, 1, GL_FLOAT, GL_FALSE,     sizeof(float) * particleDataSize, (GLvoid*)(sizeof(GL_FLOAT) * 6));
    glVertexAttribPointer(aRotT, 2, GL_FLOAT, GL_FALSE,     sizeof(float) * particleDataSize, (GLvoid*)(sizeof(GL_FLOAT) * 7));

    glVertexAttribPointer(aScale0, 2, GL_FLOAT, GL_FALSE,   sizeof(float) * particleDataSize, (GLvoid*)(sizeof(GL_FLOAT) * 9));
    glVertexAttribPointer(aScaleT, 4, GL_FLOAT, GL_FALSE,   sizeof(float) * particleDataSize, (GLvoid*)(sizeof(GL_FLOAT) * 11));

    glVertexAttribPointer(aTrans0, 1, GL_FLOAT, GL_FALSE,   sizeof(float) * particleDataSize, (GLvoid*)(sizeof(GL_FLOAT) * 15));
    glVertexAttribPointer(aTransT, 2, GL_FLOAT, GL_FALSE,   sizeof(float) * particleDataSize, (GLvoid*)(sizeof(GL_FLOAT) * 16));

    glVertexAttribPointer(aTint0, 4, GL_FLOAT, GL_FALSE,    sizeof(float) * particleDataSize, (GLvoid*)(sizeof(GL_FLOAT) * 18));
    glVertexAttribPointer(aTintT, 4, GL_FLOAT, GL_FALSE,    sizeof(float) * particleDataSize, (GLvoid*)(sizeof(GL_FLOAT) * 22));

    glVertexAttribPointer(aFrameDur, 1, GL_FLOAT, GL_FALSE, sizeof(float) * particleDataSize, (GLvoid*)(sizeof(GL_FLOAT) * 26));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    //texture
    char pathName[FILENAME_MAX] = "";
    sprintf_s(pathName, _countof(pathName), "Assets/MonkeyWalk.png");
    sf::Image loadedImage;
    loadedImage.loadFromFile(pathName);

    GLuint texture_handle;
    glGenTextures(1, &texture_handle);
    glBindTexture(GL_TEXTURE_2D, texture_handle);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, loadedImage.getSize().x, loadedImage.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, loadedImage.getPixelsPtr());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);

    //emitter variables
    std::vector<float> timeData, startData;
    std::vector<bool>  aliveData;
    std::vector<unsigned> toRemove;

    timeData.resize(maxParticles);
    aliveData.resize(maxParticles);
    toRemove.resize(maxParticles);
    startData.resize(maxParticles * particleDataSize);

    unsigned particleCount = 0;

    //emit settings
    float particleLifeTime = 3.f;
    float particleDeathTime = 5.f;

    float particlesPerSecond = 100000;
    unsigned particlesPerBurst = 1;
    float emitTimer = 0;

    //particle settings
    glm::vec2 posStart(-1, 0);
    glm::vec2 posOverTime(0, 0);
    glm::vec2 posOverTime2(0, 0);

    float rotStart = 0;
    float rotOverTime = 0;
    float rotOverTime2 = 0;

    glm::vec2 scaleStart(0.1, 0.1);
    glm::vec2 scaleOverTime(0, 0);
    glm::vec2 scaleOverTime2(0, 0);

    float transStart = 0.5f;
    float transOverTime = 0.f;
    float transOverTime2 = 0;

    glm::vec4 tintStart(0, 0, 0, 0);
    glm::vec4 tintOverTime(0, 0, 0, 0);

    float frameDuration = 0.1f;

    //start loop
    while (true)
    {
        //get dt
        float dt = deltaClock.restart().asSeconds();
        printf("%f size: %i\n", 1 / dt, particleCount);

        posStart.x += 0.1 * dt;

        //calc number of particles to emit
        unsigned particlesToEmit = 0;
        while (emitTimer <= 0)
        {
            particlesToEmit += particlesPerBurst;
            emitTimer += 1 / particlesPerSecond;
        }
        emitTimer -= dt;

        bool dataChanged = false; //any time particle data is changed, need to alert gpu of change

        //update times for current particles
        int index = -1;
        unsigned toRemoveCount = 0;
        std::for_each(timeData.begin(), timeData.begin() + particleCount, [&](float& time)
        {
            time += dt;
            ++index;

            if (time < particleLifeTime)
                return;

            //re-use now
            if (particlesToEmit > 0)
            {
                --particlesToEmit;
                dataChanged = true;

                time = 0;
                aliveData[index] = true;
                int dataIndex = index * particleDataSize;
                startData[dataIndex +  0] = posStart.x;
                startData[dataIndex +  1] = posStart.y;
                glm::vec2 velo = glm::vec2((((float)rand() / (RAND_MAX)) * 0.5 - 0.25), ((float)rand() / (RAND_MAX)) * 0.5 - 0.25);
                //startData[dataIndex +  2] = posOverTime.x;
                startData[dataIndex +  2] = velo.x;
                //startData[dataIndex +  3] = posOverTime.y;
                startData[dataIndex +  3] = velo.y;
                startData[dataIndex +  4] = posOverTime2.x;
                startData[dataIndex +  5] = posOverTime2.y;
                startData[dataIndex +  6] = rotStart;
                startData[dataIndex +  7] = rotOverTime;
                startData[dataIndex +  8] = rotOverTime2;
                startData[dataIndex +  9] = scaleStart.x;
                startData[dataIndex + 10] = scaleStart.y;
                startData[dataIndex + 11] = scaleOverTime.x;
                startData[dataIndex + 12] = scaleOverTime.y;
                startData[dataIndex + 13] = scaleOverTime2.x;
                startData[dataIndex + 14] = scaleOverTime2.y;
                startData[dataIndex + 15] = transStart;
                startData[dataIndex + 16] = transOverTime;
                startData[dataIndex + 17] = transOverTime2;
                startData[dataIndex + 18] = tintStart.r;
                startData[dataIndex + 19] = tintStart.g;
                startData[dataIndex + 20] = tintStart.b;
                startData[dataIndex + 21] = tintStart.a;
                startData[dataIndex + 22] = tintOverTime.r;
                startData[dataIndex + 23] = tintOverTime.g;
                startData[dataIndex + 24] = tintOverTime.b;
                startData[dataIndex + 25] = tintOverTime.a;
                startData[dataIndex + 26] = frameDuration;
                return;
            }

            //particle has beed dead for too long, remove from data
            if (time > particleLifeTime + particleDeathTime)
            {
                toRemove[toRemoveCount++] = index;
                dataChanged = true;
                return;
            }

            //particle needs to be made dead
            else if (time > particleLifeTime && aliveData[index])
            {
                //set all data to 0
                std::fill(startData.begin() + index * particleDataSize, startData.begin() + (index + 1) * particleDataSize, 0);
                aliveData[index] = false;
                dataChanged = true;
                return;
            }
        });

        //remove data if need be
        if (toRemoveCount)
        {
            //remove from time data
            unsigned removeIndex = 0; index = 0;
            std::remove_if(timeData.begin(), timeData.begin() + particleCount, [&](float)
            { 
                if (removeIndex >= toRemoveCount)
                    return false;
                if (index++ == toRemove[removeIndex++])
                    return true;
                return false;
            });

            //remove from alive data
            removeIndex = 0; index = 0;
            std::remove_if(aliveData.begin(), aliveData.begin() + particleCount, [&](float)
            {
                if (removeIndex >= toRemoveCount)
                    return false;
                if (index++ == toRemove[removeIndex++])
                    return true;
                return false;
            });

            //remove from start data
            removeIndex = 0; index = 0;
            std::remove_if(startData.begin(), startData.begin() + particleCount * particleDataSize, [&](float)
            {
                if (removeIndex >= toRemoveCount)
                    return false;
                if (index++ / particleDataSize == toRemove[removeIndex++])
                    return true;
                return false;
            });

            particleCount -= toRemoveCount;
        }

        //create new particles (should never happen in same loop as deleting because of recycling)
        for (unsigned i = 0; i < particlesToEmit && particleCount < maxParticles; i++)
        {
            dataChanged = true;

            timeData[particleCount] = 0;
            aliveData[particleCount] = true;
            int dataIndex = particleCount * particleDataSize;
            startData[dataIndex + 0] = posStart.x;
            startData[dataIndex + 1] = posStart.y;
            glm::vec2 velo = glm::vec2((((float)rand() / (RAND_MAX)) * 0.5 - 0.25), ((float)rand() / (RAND_MAX)) * 0.5 - 0.25);
            //startData[dataIndex +  2] = posOverTime.x;
            startData[dataIndex + 2] = velo.x;
            //startData[dataIndex +  3] = posOverTime.y;
            startData[dataIndex + 3] = velo.y;
            startData[dataIndex + 4] = posOverTime2.x;
            startData[dataIndex + 5] = posOverTime2.y;
            startData[dataIndex + 6] = rotStart;
            startData[dataIndex + 7] = rotOverTime;
            startData[dataIndex + 8] = rotOverTime2;
            startData[dataIndex + 9] = scaleStart.x;
            startData[dataIndex + 10] = scaleStart.y;
            startData[dataIndex + 11] = scaleOverTime.x;
            startData[dataIndex + 12] = scaleOverTime.y;
            startData[dataIndex + 13] = scaleOverTime2.x;
            startData[dataIndex + 14] = scaleOverTime2.y;
            startData[dataIndex + 15] = transStart;
            startData[dataIndex + 16] = transOverTime;
            startData[dataIndex + 17] = transOverTime2;
            startData[dataIndex + 18] = tintStart.r;
            startData[dataIndex + 19] = tintStart.g;
            startData[dataIndex + 20] = tintStart.b;
            startData[dataIndex + 21] = tintStart.a;
            startData[dataIndex + 22] = tintOverTime.r;
            startData[dataIndex + 23] = tintOverTime.g;
            startData[dataIndex + 24] = tintOverTime.b;
            startData[dataIndex + 25] = tintOverTime.a;
            startData[dataIndex + 26] = frameDuration;

            particleCount++;
        }

        //if data was changed, either new particles were emitted, particle died, or was deleted
        if (dataChanged)
        {
            //push the new data
            glBindBuffer(GL_ARRAY_BUFFER, data_handle);
            glBufferData(GL_ARRAY_BUFFER, maxParticles * sizeof(float) * particleDataSize, NULL, GL_STREAM_DRAW);
            glBufferSubData(GL_ARRAY_BUFFER, 0, particleCount * sizeof(float) * particleDataSize, startData.data());
        }

        //push the new times for the particles
        glBindBuffer(GL_ARRAY_BUFFER, time_handle);
        glBufferData(GL_ARRAY_BUFFER, maxParticles * sizeof(float), NULL, GL_STREAM_DRAW); //buffer orphaning
        glBufferSubData(GL_ARRAY_BUFFER, 0, particleCount * sizeof(float), timeData.data());

        //draw instances of particles
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_handle);
        glBindTexture(GL_TEXTURE_2D, texture_handle);
        glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, particleCount);

        window->display();
        glClear(GL_COLOR_BUFFER_BIT);

        //things to start with that are the same until recycled
        //position_0 + position/time + position/time^2 (vec2) * 3  = 6
        //rotation_0 + rotation/time + rotation/time^2 (float) * 3 = 3
        //scale_0 + scale/time + scale/time^2          (vec2) * 3  = 6
        //transp_0 + transp/time + transp/time^2       (float) * 3 = 3
        //tint_0 + tint/time                           (vec4) * 2  = 8
        //frameduration                                (float)     = 1
        //                                                          27 floats per particle
        //
        //could be all the same for every particle, any number of them could be different
        //      maybe change based on how many get read in per element?
    }

    delete window;
    glDeleteProgram(program);
    return EXIT_SUCCESS;
}*/