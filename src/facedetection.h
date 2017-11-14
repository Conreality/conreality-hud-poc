static cv::String file1 = "./haarcascades/haarcascade_frontalface_alt.xml";
static cv::String file2 = "./haarcascades/haarcascade_eye_tree_eyeglasses.xml";

static void initializeFace(std::vector<cv::Rect> faces);
static void updateFaces(std::vector<cv::Rect> faces);
static void createFaces(std::vector<cv::Rect> faces);
static void decrementFaces();
static void removeCopies();
static void drawCircles(cv::Mat grayFrame, cv::Mat frame);

struct faceStruct {
  int locationX;
  int locationY;
  int height;
  int width;

  bool toBeRemoved;
  bool hasBeenUpdated;

  int count;
};

static cv::CascadeClassifier face_cascade, eyes_cascade;
std::vector<std::unique_ptr<faceStruct>> faceVector;

void detectFaces(cv::Mat frame) {
  cv::Mat orig = frame.clone();
  cv::Mat grayFrame;
  int radius;

  cvtColor(orig, grayFrame, CV_BGR2GRAY);
  equalizeHist(grayFrame, grayFrame);

  std::vector<cv::Rect> faces;

  face_cascade.detectMultiScale(grayFrame, faces, 1.3,2,0,cv::Size(30,30));

  if (faceVector.size() == 0 && faces.size() > 0) {
    initializeFace(faces);
  }

  updateFaces(faces);
  createFaces(faces);
  decrementFaces();
  removeCopies();
  drawCircles(grayFrame, frame);
}

void initializeFace(std::vector<cv::Rect> faces) {
    for (int i = 0; i < faces.size(); i++) {
      std::unique_ptr<faceStruct> ptr (new faceStruct());
      ptr->locationX = faces[i].x;
      ptr->locationY = faces[i].y;
      ptr->width = faces[i].width;
      ptr->height = faces[i].height;
      ptr->count = 1;
      ptr->hasBeenUpdated = true;
      ptr->toBeRemoved = false;
      faceVector.push_back(move(ptr));
      break; //just need one
    }
}

void updateFaces(std::vector<cv::Rect> faces) {
  if (faceVector.size() > 0) {
    for (auto itr = faceVector.begin(); itr != faceVector.end(); ++itr) {
      (*itr)->hasBeenUpdated = false;
      for (int i = 0; i < faces.size(); i++) {
        if (abs(faces[i].x - (*itr)->locationX) < 50 && abs(faces[i].y - (*itr)->locationY) < 50) {
          if ((*itr)->count < 6) {
            (*itr)->count += 1;
          }
          (*itr)->hasBeenUpdated = true;
          if (abs(faces[i].x - (*itr)->locationX) > 15) { (*itr)->locationX = faces[i].x; }
          if (abs(faces[i].y - (*itr)->locationY) > 15) { (*itr)->locationY = faces[i].y; }
          if (abs(faces[i].width - (*itr)->width) > 30) { (*itr)->width = faces[i].width; }
          if (abs(faces[i].height - (*itr)->height) > 30) { (*itr)->height = faces[i].height; }

          break; //match found
        }
      }
    }
  }
}

void createFaces(std::vector<cv::Rect> faces) {
  if (faceVector.size() > 0) {
    for (int i = 0; i < faces.size(); i++) {
      bool toAdd = true;
      for (auto itr = faceVector.begin(); itr != faceVector.end(); ++itr) {
        if (abs(faces[i].x - (*itr)->locationX) < 50 && abs(faces[i].y - (*itr)->locationY) < 50) {
          toAdd = false;
        }
      } // append after iterating
      if (toAdd) {
        std::unique_ptr<faceStruct> ptr (new faceStruct());
        ptr->locationX = faces[i].x;
        ptr->locationY = faces[i].y;
        ptr->width = faces[i].width;
        ptr->height = faces[i].height;
        ptr->count = 1;
        ptr->hasBeenUpdated = true;
        ptr->toBeRemoved = false;
        faceVector.push_back(move(ptr));
      }
    }
  }
}

void decrementFaces() {
  if (faceVector.size() > 0) {
    for (auto itr = faceVector.begin(); itr != faceVector.end(); ++itr) {
      if ((*itr)->hasBeenUpdated == false && (*itr)->count > 0) {
        (*itr)->count -= 1;
      }
    }
    for (auto itr = faceVector.begin(); itr != faceVector.end(); ++itr) {
      if ((*itr)->hasBeenUpdated == false && (*itr)->count == 0) {
        (*itr)->toBeRemoved = true;
      }
    }
  }
}

void removeCopies() {
  if (faceVector.size() > 0) {
    for (auto bItr = faceVector.begin(); bItr != faceVector.end(); ++bItr) {
      for (auto fItr = faceVector.begin(); fItr != faceVector.end(); ++fItr) {
        if ((*bItr) != (*fItr)) {
          if (abs((*bItr)->locationX - (*fItr)->locationX) < 20 && abs((*bItr)->locationY - (*fItr)->locationY) < 20) {
            (*bItr)->toBeRemoved = true;
            break;
          }
        }
      }
    }
  }
  
  if (faceVector.size() > 0) {
    for (auto itr = faceVector.begin(); itr != faceVector.end(); ++itr) {
      if ((*itr)->toBeRemoved == true) {
        itr = faceVector.erase(itr);
        break;
      }
    }
  }
}

void drawCircles(cv::Mat grayFrame, cv::Mat frame) {
  std::vector<cv::Rect> eyes;
  if (faceVector.size() > 0) {
    for (auto itr = faceVector.begin(); itr != faceVector.end(); ++itr) {
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
}
