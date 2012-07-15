#include "Shape.h"

using namespace std;
using namespace ci;

namespace chronotext
{
    Shape::Shape(ShapeStyleRef style)
    {
        setLocation(0, 0);
        setPadding(style->paddingLeft, style->paddingTop, style->paddingRight, style->paddingBottom);
        setMargin(style->marginLeft, style->marginTop, style->marginRight, style->marginBottom);
        
        if (style->width > 0)
        {
            setWidth(style->width);
        }
        else
        {
            setAutoWidth(style->autoWidth);
            width = 0;
        }
        
        if (style->height > 0)
        {
            setHeight(style->height);
        }
        else
        {
            setAutoHeight(style->autoHeight);
            height = 0;
        }

        visible = style->visible;
    }
    
    void Shape::setLocation(float x, float y)
    {
        this->x = x;
        this->y = y;
    }
    
    void Shape::setWidth(float newWidth)
    {
        width = newWidth;
        autoWidth = false;
    }
    
    void Shape::setHeight(float newHeight)
    {
        height = newHeight;
        autoHeight = false;
    }
    
    void Shape::setAutoWidth(bool newAuto)
    {
        autoWidth = newAuto;
    }
    
    void Shape::setAutoHeight(bool newAuto)
    {
        autoHeight = newAuto;
    }
    
    void Shape::setBounds(const Rectf &bounds)
    {
        x = bounds.x1;
        y = bounds.y1;
        
        setWidth(bounds.getWidth());
        setHeight(bounds.getHeight());
    }
    
    void Shape::setPadding(float left, float top, float right, float bottom)
    {
        paddingLeft = left;
        paddingTop = top;
        paddingRight = right;
        paddingBottom = bottom;
    }
    
    void Shape::setMargin(float left, float top, float right, float bottom)
    {
        marginLeft = left;
        marginTop = top;
        marginRight = right;
        marginBottom = bottom;
    }
    
    ci::Vec2f Shape::getLocation()
    {
        return Vec2f(x, y);
    }
    
    float Shape::getWidth()
    {
        return width;
    }
    
    float Shape::getHeight()
    {
        return height;
    }
    
    ci::Rectf Shape::getBounds()
    {
        return Rectf(x, y, x + getWidth(), y + getHeight());
    }
    
    vector<Touchable*> Shape::getTouchables()
    {
        return vector<Touchable*>();
    }
}