#include "chronotext/android/cinder/CinderDelegate.h"
#include "chronotext/InputSource.h"

using namespace ci;
using namespace ci::app;
using namespace std;

enum
{
    EVENT_ATTACHED = 1,
    EVENT_DETACHED,
    EVENT_PAUSED,
    EVENT_RESUMED,
    EVENT_SHOWN,
    EVENT_HIDDEN,
    EVENT_DESTROYED
};

#define GRAVITY_EARTH 9.80665f

/*
 * CALLED ON THE RENDERER'S THREAD FROM chronotext.android.gl.GLRenderer.onSurfaceCreated()
 */
void CinderDelegate::launch(AAssetManager *assetManager, JavaVM *javaVM, jobject javaListener)
{
    mJavaVM = javaVM;
    mJavaListener = javaListener;

    // ---

    InputSource::setAndroidAssetManager(assetManager);

    // ---

    ALooper *looper = ALooper_forThread();

    if (!looper)
    {
        looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
    }

    mSensorManager = ASensorManager_getInstance();
    mAccelerometerSensor = ASensorManager_getDefaultSensor(mSensorManager, ASENSOR_TYPE_ACCELEROMETER);
    mSensorEventQueue = ASensorManager_createEventQueue(mSensorManager, looper, 3, NULL, NULL/*sensorEventCallback, this*/); // WOULD BE BETTER TO USE A CALL-BACK, BUT IT'S NOT WORKING
}

void CinderDelegate::processSensorEvents()
{
    ASensorEvent event;

    while (ASensorEventQueue_getEvents(mSensorEventQueue, &event, 1) > 0)
    {
      if (event.type == ASENSOR_TYPE_ACCELEROMETER)
      {
          float x = event.acceleration.x;
          float y = event.acceleration.y;
          float z = event.acceleration.z;

          /*
           * APPLYING THE EVENTUAL ORIENTATION FIX
           */
          if (mAccelerometerRotation == ACCELEROMETER_ROTATION_LANDSCAPE)
          {
              float tmp = x;
              x = -y;
              y = +tmp;
          }
          else if (mAccelerometerRotation == ACCELEROMETER_ROTATION_PORTRAIT)
          {
              float tmp = x;
              x = +y;
              y = -tmp;
          }

          /*
           * FOR CONSISTENCY WITH iOS
           */
          x /= -GRAVITY_EARTH;
          y /= +GRAVITY_EARTH;
          z /= +GRAVITY_EARTH;

          accelerated(x, y, z);
      }
    }
}

void CinderDelegate::accelerated(float x, float y, float z)
{
    Vec3f acceleration(x, y, z);
    Vec3f filtered = mLastAccel * (1 - mAccelFilterFactor) + acceleration * mAccelFilterFactor;

    AccelEvent event(filtered, acceleration, mLastAccel, mLastRawAccel);
    sketch->accelerated(event);

    mLastAccel = filtered;
    mLastRawAccel = acceleration;
}

void CinderDelegate::init(int width, int height, int accelerometerRotation)
{
    mWidth = width;
    mHeight = height;
    mAccelerometerRotation = accelerometerRotation;

    sketch->setup(false);
    sketch->resize(ResizeEvent(Vec2i(mWidth, mHeight)));
}

void CinderDelegate::draw()
{
    /*
     * WOULD BE BETTER TO USE A CALL-BACK, BUT IT'S NOT WORKING
     */
    processSensorEvents();

    sketch->update();
    sketch->draw();
    mFrameCount++;
}

void CinderDelegate::event(int id)
{
    switch (id)
    {
        case EVENT_ATTACHED:
        case EVENT_SHOWN:
	    mFrameCount = 0;
            mTimer.start();
            sketch->start(CinderSketch::FLAG_FOCUS_GAIN);
            break;
            
        case EVENT_RESUMED:
            mFrameCount = 0;
            mTimer.start();
            
            /*
             * ASSERTION: THE GL CONTEXT HAS JUST BEEN RE-CREATED
             */
            sketch->setup(true);
            sketch->resize(ResizeEvent(Vec2i(mWidth, mHeight)));
            
            sketch->start(CinderSketch::FLAG_APP_RESUME);
            break;
            
        case EVENT_DETACHED:
        case EVENT_HIDDEN:
            mTimer.stop();
            sketch->stop(CinderSketch::FLAG_FOCUS_LOST);
            break;
            
        case EVENT_PAUSED:
            mTimer.stop();
            sketch->stop(CinderSketch::FLAG_APP_PAUSE);
            break;

        case EVENT_DESTROYED:
            ASensorManager_destroyEventQueue(mSensorManager, mSensorEventQueue);

            sketch->shutdown();
            delete sketch;
            break;
    }
}

void CinderDelegate::addTouch(float x, float y)
{
    sketch->addTouch(0, x, y);
}

void CinderDelegate::updateTouch(float x, float y)
{
    sketch->updateTouch(0, x, y);
}

void CinderDelegate::removeTouch(float x, float y)
{
    sketch->removeTouch(0, x, y);
}

void CinderDelegate::enableAccelerometer(float updateFrequency, float filterFactor)
{
    mAccelFilterFactor = filterFactor;

    int delay = 1000000 / updateFrequency;
    int min = ASensor_getMinDelay(mAccelerometerSensor);

    if (delay < min)
    {
        delay = min;
    }

    ASensorEventQueue_enableSensor(mSensorEventQueue, mAccelerometerSensor);
    ASensorEventQueue_setEventRate(mSensorEventQueue, mAccelerometerSensor, delay);
}

void CinderDelegate::disableAccelerometer()
{
    ASensorEventQueue_disableSensor(mSensorEventQueue, mAccelerometerSensor);
}

double CinderDelegate::getElapsedSeconds() const
{
    return mTimer.getSeconds();
}

uint32_t CinderDelegate::getElapsedFrames() const
{
    return mFrameCount;
}

int CinderDelegate::getWindowWidth()
{
    return mWidth;
}

int CinderDelegate::getWindowHeight()
{
    return mHeight;
}

Vec2f CinderDelegate::getWindowSize()
{
    return Vec2f(mWidth, mHeight);
}

float CinderDelegate::getWindowAspectRatio()
{
    return mWidth / (float)mHeight;
}

Area CinderDelegate::getWindowBounds() const
{
    return Area(0, 0, mWidth, mHeight);
}

ostream& CinderDelegate::console()
{
    if (!mOutputStream)
    {
        mOutputStream = shared_ptr<cinder::android::dostream>(new android::dostream);
    }
    
    return *mOutputStream;
}

void CinderDelegate::sendMessage(int what, const string &body)
{
    JNIEnv *env;
    mJavaVM->GetEnv((void**)&env, JNI_VERSION_1_4);
   
    jclass cls = env->GetObjectClass(mJavaListener);
    jmethodID method = env->GetMethodID(cls, "handleMessage", "(ILjava/lang/String;)V");
    env->CallVoidMethod(mJavaListener, method, what, env->NewStringUTF(body.c_str()));
}

void CinderDelegate::handleMessage(int what, const string &body)
{}
