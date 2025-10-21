#ifndef Mouse_hpp
#define Mouse_hpp

namespace RC::Mouse {

extern bool buttonLeft;
extern bool buttonRight;
extern float movedX;
extern float scrolledY;

void leftButtonPressed();
void leftButtonReleased();
void rightButtonPressed();
void rightButtonReleased();
void moved(float dx, float dy);
void scrolled(float dx, float dy);

} // namespace RC::Mouse

#endif /* Mouse_hpp */
