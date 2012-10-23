#pragma once

#include "chronotext/incubator/particles/Particle.h"
#include "chronotext/incubator/particles/EmitterParams.h"
#include "chronotext/incubator/particles/EmitterController.h"
#include "chronotext/maths/Random.h"
#include "chronotext/time/MasterClock.h"
#include "chronotext/texture/TextureAtlas.h"

namespace chronotext
{
    class EmitterData
    {
    public:
        virtual ~EmitterData() {}
    };
    
    typedef boost::shared_ptr<class Emitter> EmitterRef;

    class Emitter
    {
    protected:
        EmitterParams params;
        
        EmitterController *controller;
        bool controllerIsOwned;
        
        EmitterData *data;
        bool dataIsOwned;
        
        std::vector<Sprite*> sprites;
        std::list<Particle> particles;
        
        bool ended;
        bool spawnEnded;
        bool spawnEnabled;
        
        float accum;
        int total;
        
    public:
        Clock clock;
        ci::Rand random;

        ci::Vec2f position;

        Emitter(MasterClock *masterClock, TextureAtlas *atlas, const EmitterParams &params, uint32_t seed = 214);
        ~Emitter();

        virtual void setController(EmitterController *controller, bool controllerIsOwned);
        virtual void setData(EmitterData *data, bool dataIsOwned);
        
        virtual EmitterData* getData();
        virtual bool hasEnded();

        virtual void update(float dt);
        virtual void draw(const ci::Vec2f &scale);

        void setSpawnEnabled(bool b);
        virtual void spawnParticle(double now);
        
        virtual void accumulateForces();
        virtual void integrate(float dt);
        virtual void satisfyConstraints();
    };
}

namespace chr = chronotext;