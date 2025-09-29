//
//  Primitives.hpp
//  RayCasting
//
//  Created by Aleksei Baryzhikov on 06.09.25.
//

#ifndef Primitives_hpp
#define Primitives_hpp

namespace RC {

void line(float x0, float y0, float x1, float y1);

void box(float x, float y, float w, float h);

void boxFill(float x, float y, float w, float h);

} // namespace RC

#endif /* Primitives_hpp */
