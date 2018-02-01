#ifndef EYEFINDER_H
#define EYEFINDER_H

#include <dlib/opencv.h>
#include <opencv2/highgui/highgui.hpp>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
#include <dlib/gui_widgets.h>

#include "opencv2/imgproc/imgproc.hpp"

#include <vector>
#include <tuple>
#include <utility>
#include <string>

// C headers
// linke with -lpthread
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
// exit()
#include <stdlib.h>
// sleep()
#include <unistd.h>


#define DEBUG 0
#define MACRO_START (begin=std::chrono::steady_clock::now())
#define MACRO_END   (end  =std::chrono::steady_clock::now())
#define MACRO_P_DIFF(MSG) (std::cout<<MSG\
                           <<std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count()\
                           <<std::endl)
namespace _EF_ {

  class EyeFinder final {

  public:

    EyeFinder(void);
    ~EyeFinder(void);
    // Deleted so blocked from use
    EyeFinder(const EyeFinder& other) = delete;
    EyeFinder(EyeFinder&& other) = delete;
    EyeFinder& operator=(const EyeFinder& other) = delete;
    EyeFinder& operator=(EyeFinder&& other) = delete;

    int start(void);

  private:

    const bool showmain = true;
    const bool showeyes = true;
    const bool clear = true;
    static const unsigned int NUM_WINS = 3;

    cv::VideoCapture cap;
    dlib::image_window win[NUM_WINS];

    sem_t * sem;
    const char * sem_name = "/capstone";

    int shmid;
    void * shared_memory;
    const key_t key = 123456;
    const unsigned int shared_size = 100;

    // std::vector<std::pair<long, long>> facial_features_vec;
    // change to python list later with boost, look below in start()

    void preCalculationPoints(std::vector<std::pair<long,long>>& facial_features_vec);
    std::tuple<long, long, long, long> setMinAndMax(int start,
                                                    int end,
                                                    std::vector<dlib::full_object_detection>& shapes);
    cv::Rect getROI(std::tuple<long,long,long,long>& tp, cv::Mat frame);
    bool anyWindowsClosed(void);
    void calculateFaceAngles(void);
    void calculatePupils(void);
    void writeFacialFeaturesToShm(const std::vector<std::pair<long,long>>& facial_features_vec);
    void writeBadFacialFeaturesToShm(void);
  };
  // Won't be able to work since EyeFinder is Final. Good!
  // struct DerivedEyeFinder : EyeFinder {};
};
#endif
