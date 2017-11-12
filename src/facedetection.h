static cv::String file1 = "./haarcascades/haarcascade_frontalface_alt.xml";
static cv::String file2 = "./haarcascades/haarcascade_eye_tree_eyeglasses.xml";

struct faceStruct {
  int locationX;
  int locationY;
  int height;
  int width;

  int count;
};

static cv::CascadeClassifier face_cascade, eyes_cascade;
std::vector<std::unique_ptr<faceStruct>> facesVector;

void detectFaces(cv::Mat frame) {
  cv::Mat orig = frame.clone();
  cv::Mat grayFrame;
  int radius;

  cvtColor(orig, grayFrame, CV_BGR2GRAY);
  equalizeHist(grayFrame, grayFrame);

  std::vector<cv::Rect> faces, eyes;

  face_cascade.detectMultiScale(grayFrame, faces, 1.3,2,0,cv::Size(30,30));

  for (int i = 0; i < faces.size(); i++) {

    if(facesVector.size() == 0) {
      std::unique_ptr<faceStruct> ptr (new faceStruct());
      ptr->locationX = faces[i].x;
      ptr->locationY = faces[i].y;
      ptr->width = faces[i].width;
      ptr->height = faces[i].height;
      ptr->count = 1;
      facesVector.push_back(move(ptr));
    } else {
      for (auto itr = facesVector.begin(); itr != facesVector.end(); ++itr) {

        if (abs(faces[i].x - (*itr)->locationX) < 50 && abs(faces[i].y - (*itr)->locationY) < 50) {
          if ((*itr)->count < 6) {
            (*itr)->count += 1;
          }
          if (abs(faces[i].x - (*itr)->locationX) > 10) { (*itr)->locationX = faces[i].x; }
          if (abs(faces[i].y - (*itr)->locationY) > 10) { (*itr)->locationY = faces[i].y; }
          if (abs(faces[i].width - (*itr)->width) > 20) { (*itr)->width = faces[i].width; }
          if (abs(faces[i].height - (*itr)->height) > 20) { (*itr)->height = faces[i].height; }
          
          break;

        } else if (abs(faces[i].x - (*itr)->locationX) > 50 || abs(faces[i].y - (*itr)->locationY) > 50) {

          if ((*itr)->count > 0) {
            (*itr)->count -= 1;

          } else if ((*itr)->count == 0) {
            itr = facesVector.erase(itr);
            break;
          }
        }
      }
    } 
  }

  for (auto itr = facesVector.begin(); itr != facesVector.end(); ++itr) {
    cv::Rect tmp((*itr)->locationX,
                  (*itr)->locationY,
                  (*itr)->width,
                  (*itr)->height);
    cv::Mat face = grayFrame(tmp);
    eyes_cascade.detectMultiScale(face, eyes, 1.3,1,0);

    if(eyes.size() > 0) {
        cv::Point center( (*itr)->locationX + (*itr)->width/2,
                          (*itr)->locationY + (*itr)->height/2 );
      if((*itr)->count > 4) {
        cv::circle(frame, center, (*itr)->width/2, cv::Scalar(255,0,0), 2, 8, 0);
      }
    }
  }
}
