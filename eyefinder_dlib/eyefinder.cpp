// The contents of this file are in the public domain. See LICENSE_FOR_EXAMPLE_PROGRAMS.txt
/*

    This example program shows how to find frontal human faces in an image and
    estimate their pose.  The pose takes the form of 68 landmarks.  These are
    points on the face such as the corners of the mouth, along the eyebrows, on
    the eyes, and so forth.


    This example is essentially just a version of the face_landmark_detection_ex.cpp
    example modified to use OpenCV's VideoCapture object to read from a camera instead
    of files.


    Finally, note that the face detector is fastest when compiled with at least
    SSE2 instructions enabled.  So if you are using a PC with an Intel or AMD
    chip then you should enable at least SSE2 instructions.  If you are using
    cmake to compile this program you can enable them by using one of the
    following commands when you create the build project:
        cmake path_to_dlib_root/examples -DUSE_SSE2_INSTRUCTIONS=ON
        cmake path_to_dlib_root/examples -DUSE_SSE4_INSTRUCTIONS=ON
        cmake path_to_dlib_root/examples -DUSE_AVX_INSTRUCTIONS=ON
    This will set the appropriate compiler options for GCC, clang, Visual
    Studio, or the Intel compiler.  If you are using another compiler then you
    need to consult your compiler's manual to determine how to enable these
    instructions.  Note that AVX is the fastest but requires a CPU from at least
    2011.  SSE4 is the next fastest and is supported by most current machines.
*/

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

namespace EyeFinder {
  #define DEBUG 0
  #define MACRO_START (begin=std::chrono::steady_clock::now())
  #define MACRO_END   (end  =std::chrono::steady_clock::now())
  #define MACRO_P_DIFF(MSG) (std::cout<<MSG\
                             <<std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count()\
                             <<std::endl)

	key_t key = 123456;
	unsigned int shared_size = 100;

  struct EyeFinder final {
  private:
    const bool showmain = true;
    const bool showeyes = true;
    const bool clear = true;
    static const unsigned int NUM_WINS = 3;

    sem_t * sem;
    int shmid;
  	void * shared_memory;

    cv::VideoCapture cap;
    dlib::image_window win[NUM_WINS];
    // std::vector<std::pair<long, long>> facial_features_vec;
    // change to python list later with boost, look below in start()

    const char * sem_name = "/capstone";

  public:
    EyeFinder(void) : cap(0) {
      // set the buffersize of cv::VideoCapture -> cap
      cap.set(CV_CAP_PROP_BUFFERSIZE, 3);

      // Initialize the Semaphore (POSIX semaphore)
      sem = sem_open(sem_name, O_CREAT|O_EXCL, 0666, 1);
      if (sem == SEM_FAILED) {
        std::cout<<"*** *** sem_open failed!!!"<<std::endl;
        // sem_close(sem); // Neccessary???
        sem = sem_open(sem_name, O_CREAT, 0666, 1);
        //sem_unlink(sem_name);
        sem_close(sem);
        exit(1);
      }
      else {
        printf("sem: %p\n", sem);
      }

      // Initialize the Shared Memory (System V shared Memory)
      // Setup shared memory
    	if ((shmid = shmget(key, shared_size, IPC_EXCL | IPC_CREAT | 0666)) < 0)
    	{std::cout<<"*** *** shmget() failed!!!"<<std::endl;exit(1);}
    	else { // GOLDEN!!!
    		std::cout << "shmid: " << shmid << std::endl;
    	}
      // Attached shared memory
    	if ((shared_memory = (char *)shmat(shmid, NULL, 0)) == (char *) -1)
    	{printf("Error attaching shared memory id");exit(1);}
    	else { // GOLDEN!!!
    		std::cout << "shared_memory: " << (unsigned long)shared_memory << std::endl;
    	}
    }
    ~EyeFinder(void) {
      // Free the Semaphore
      sem_unlink(sem_name);
      sem_close(sem);
      // Free the Shared Memory
      shmdt(shared_memory);
    	shmctl(shmid, IPC_RMID, NULL);

      // Bye message
      std::cout<<"\n\nBye~~~!\n"<<std::endl;
    }

    // Deleted so blocked from use
    EyeFinder(const EyeFinder& other) = delete;
    EyeFinder(EyeFinder&& other) = delete;
    EyeFinder& operator=(const EyeFinder& other) = delete;
    EyeFinder& operator=(EyeFinder&& other) = delete;

    // *****
    // start
    int start(void) {
      try
      {
          std::chrono::steady_clock::time_point begin, end;

          if (!cap.isOpened()) {std::cerr << "Unable to connect to camera" << std::endl; return 1;}

          // Load face detection and pose estimation models.
          dlib::frontal_face_detector detector = dlib::get_frontal_face_detector();
          dlib::shape_predictor pose_model;
          dlib::deserialize("shape_predictor_68_face_landmarks.dat") >> pose_model;

          // Grab and process frames until the main window is closed by the user.
          int i = 0;
          while(!anyWindowsClosed() && i++ < 100) //|| !(showmain | showeyes)
          {
            // Grab a frame
            MACRO_START;
            cv::Mat temp;
            if (!cap.read(temp)) {break;}
            MACRO_END;
            MACRO_P_DIFF("cap.read(temp) = ");

            // Shrink the frame size
            cv::resize(temp, temp,  cv::Size(), 0.5, 0.5);

            // Turn OpenCV's Mat into something dlib can deal with.  Note that this just
            // wraps the Mat object, it doesn't copy anything.  So cimg is only valid as
            // long as temp is valid.  Also don't do anything to temp that would cause it
            // to reallocate the memory which stores the image as that will make cimg
            // contain dangling pointers.  This basically means you shouldn't modify temp
            // while using cimg.
            dlib::cv_image<dlib::bgr_pixel> cimg(temp);

            // Detect faces
            MACRO_START;
            std::vector<dlib::rectangle> faces = detector(cimg);
            MACRO_END;
            MACRO_P_DIFF("detector(cimg) = ");

            // Find the pose of each face. (Only the first)
            MACRO_START;
            std::vector<dlib::full_object_detection> shapes;
            for (unsigned long i = 0; i<1 && i<faces.size(); ++i)
                shapes.push_back(pose_model(cimg, faces[i]));
            MACRO_END;
            MACRO_P_DIFF("shapes.push_back(...) = ");

            // Display it all on the screen
            std::vector<dlib::image_window::overlay_line> rfd_res = render_face_detections(shapes);

            if (shapes.size()) { // need to check
              // left eye
              std::tuple<long,long,long,long> l_tp = EyeFinder::setMinAndMax(36,41,shapes);
              // right eye
              std::tuple<long,long,long,long> r_tp = EyeFinder::setMinAndMax(42,47,shapes);

              // ROI for left eye
              cv::Rect roi_l = EyeFinder::getROI(l_tp, temp);
              // ROI for right eye
              cv::Rect roi_r = EyeFinder::getROI(r_tp, temp);

              // Display eye tracking on the screen
              cv::Mat roi_l_mat, roi_r_mat;
              roi_l_mat = temp(roi_l);
              roi_r_mat = temp(roi_r);
              dlib::cv_image<dlib::bgr_pixel> l_eye_cimg(roi_l_mat);
              dlib::cv_image<dlib::bgr_pixel> r_eye_cimg(roi_r_mat);
              if (showeyes) {
                win[1].set_image(l_eye_cimg);
                win[2].set_image(r_eye_cimg);
              }
            }

            if (showmain) {
              if (clear) win[0].clear_overlay();
              win[0].set_image(cimg);
              win[0].add_overlay(rfd_res);
            }

            if (shapes.size()) {
              std::vector<std::pair<long, long>> facial_features_vec;

              // Find the face angle
              calculateFaceAngles();

              // Find the pupil
              calculatePupils();

              // Remove this *** *** *** *** ***
              // Need to create the facial_features_vec...
              for (long i=0; i<60; ++i) facial_features_vec.push_back(std::make_pair<long,long>(i*100,i*100+1));
              // Write out the Facial Features
              writeFacialFeaturesToShm(facial_features_vec);
            }
            else {
              writeBadFacialFeaturesToShm();
            }
            std::cout<<std::endl;
          }
      }
      catch(dlib::serialization_error& e)
      {
          std::cout << "You need dlib's default face landmarking model file to run this example." << std::endl;
          std::cout << "You can get it from the following URL: " << std::endl;
          std::cout << "   http://dlib.net/files/shape_predictor_68_face_landmarks.dat.bz2" << std::endl;
          std::cout << std::endl << e.what() << std::endl;
      }
      catch(std::exception& e)
      {
          std::cout << e.what() << std::endl;
      }
      return 0;
    }

    // *****
    // preCalculationPoints
    //  0-11 = x, y coordinates of left eye points
    //  12-23 = x, y coordinates of right eye points
    void preCalculationPoints(std::vector<std::pair<long,long>>& facial_features_vec) {

    }

    // *****
    // setMinAndMax
    inline std::tuple<long, long, long, long>
    setMinAndMax(int start, int end,
                 std::vector<dlib::full_object_detection>& shapes) {
      // min_x, min_y, max_x, max_y
      std::tuple<long,long,long,long> tp{LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN};

      for (int i=start; i<=end; ++i) {
        auto x = shapes[0].part(i).x();
        auto y = shapes[0].part(i).y();
        std::get<0>(tp) = std::min(std::get<0>(tp), x);
        std::get<1>(tp) = std::min(std::get<1>(tp), y);
        std::get<2>(tp) = std::max(std::get<2>(tp), x);
        std::get<3>(tp) = std::max(std::get<3>(tp), y);
      }

      return tp;
    }

    // *****
    // getROI
    inline cv::Rect getROI(std::tuple<long,long,long,long>& tp, cv::Mat frame) {
      #if DEBUG
      std::cout << std::get<0>(tp) << " " << std::get<1>(tp) << " "
                << std::get<2>(tp) << " " << std::get<3>(tp) << std::endl;
      #endif
      auto start_x = std::max(std::get<0>(tp)-10, long(0));
      auto start_y = std::max(std::get<1>(tp)-10, long(0));
      auto size_x  = std::min(std::get<2>(tp)-std::get<0>(tp)+15, frame.cols-start_x);
      auto size_y  = std::min(std::get<3>(tp)-std::get<1>(tp)+15, frame.rows-start_y);
      return cv::Rect(start_x, start_y, size_x, size_y);
    }

    // *****
    // anyWindowsClosed
    inline bool anyWindowsClosed(void) {
      for (int i=0; i<NUM_WINS; ++i)
        if (win[i].is_closed()) return true;
      return false;
    }

    // ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****
    // ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****
    //
    // Calculation!!! (finding face angles, pupils, and writing out to shared memory)
    //
    // ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****
    // ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****

    // *****
    // calculateFaceAngles()
    void calculateFaceAngles(void) {}

    // *****
    // calculatePupils() a.k.a. Timm Barthe Algorithm
    void calculatePupils(void) {}

    // *****
    // writeFacialFeaturesToShm()
    //  Will be to shared memory with semaphore synchronization
    //  0-11 = x, y coordinates of left eye points
    //  12-23 = x, y coordinates of right eye points
    //  24, 25 = x, y coordinates of left eye center (timm)
    //  26, 27 = x, y coordinates of right eye center (timm)
    //  28-29 = face angles (another code)
    void writeFacialFeaturesToShm(const std::vector<std::pair<long,long>>& facial_features_vec) {
      for (const auto pr : facial_features_vec) {
        //std::cout<<pr.first<<","<<pr.second<<" ";
      }
      //std::cout<<std::endl;
    }
    void writeBadFacialFeaturesToShm(void) {}

  };

  // Won't be able to work since EyeFinder is Final. Good!
  // struct DerivedEyeFinder : EyeFinder {};
};


int main()
{
  struct EyeFinder::EyeFinder ef;
  ef.start();
  return 0;
}
