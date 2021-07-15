#include "Engine.h"
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <vector>
#include <random>

//
//  You are free to modify this file
//

//  is_key_pressed(int button_vk_code) - check if a key is pressed,
//                                       use keycodes (VK_SPACE, VK_RIGHT, VK_LEFT, VK_UP, VK_DOWN, 'A', 'B')
//
//  get_cursor_x(), get_cursor_y() - get mouse cursor position
//  is_mouse_button_pressed(int button) - check if mouse button is pressed (0 - left button, 1 - right button)
//  clear_buffer() - set all pixels in buffer to 'black'
//  is_window_active() - returns true if window is active
//  schedule_quit_game() - quit game after act()
inline double distance(int x_1, int y_1, int x_2, int y_2) { return pow(pow(x_1 - x_2, 2) + pow(y_1 - y_2, 2), 0.5); }


class Buffer_colour
{
private:
    unsigned char* blue_byte;
    unsigned char* green_byte;
    unsigned char* red_byte;

    void set_buffer(uint32_t& pixel)
    {
        blue_byte = (unsigned char*)(&pixel);
        green_byte = blue_byte + 1;
        red_byte = blue_byte + 2;
    }
public:
    void set_colours(uint32_t& pixel, int red, int green, int blue)
    {
        set_buffer(pixel);
        *(blue_byte) = blue;
        *(green_byte) = green;
        *(red_byte) = red;
    }
} buffer_colour;

double boundary_condition(double position, int limit)
{
    int pos = position / limit;
    position -= pos * limit;
    if (position < 0)
    {
        position += limit;
    }
    return position;
}

class Space_object
{
protected:
    double m_x;
    double m_y;
    double m_angle;
    double m_speed;
    double m_health;
    int m_size;
public:
    Space_object(double x, double y, double angle, double speed, double health, int size)
        : m_x(x), m_y(y), m_angle(angle), m_speed(speed), m_health(health), m_size(size) {}
    virtual ~Space_object() = default;

    virtual void draw() {};

    virtual bool alive()
    {
        if (m_health > 0)
            return true;
        return false;
    }

    virtual void move(float dt)
    {
        m_x += m_speed * cos(m_angle) * dt;
        m_x = boundary_condition(m_x, SCREEN_WIDTH);

        m_y += m_speed * sin(m_angle) * dt;
        m_y = boundary_condition(m_y, SCREEN_HEIGHT);
    }

    virtual void hit(double dt)
    {
        m_health -= 100*dt;
    }

    double get_x() { return m_x; }
    double get_y() { return m_y; }
    double get_angle() { return m_angle; }
    double get_speed() { return m_speed; }
    int get_size() { return m_size; }
        

};


class My_ship: public Space_object
{
private:
    const double max_speed = 300;
    const double max_health = 50;

public:
    My_ship(double x = SCREEN_WIDTH / 2, double y = SCREEN_HEIGHT / 2, double angle = 0, double speed = 0, double health = 50, int size = 10)
        : Space_object(x, y, angle, speed, health, size) {}

    void draw()
    {
        int x = int(m_x);
        int y = int(m_y);
        int height;
        int width;
        for (auto i = x - m_size; i <= x + m_size; ++i)
        {
            for (auto j = y - m_size; j <= y + m_size; ++j)
            {
                if (distance(x, y, i, j) < m_size)
                {
                    height = boundary_condition(j, SCREEN_HEIGHT);
                    width = boundary_condition(i, SCREEN_WIDTH);
                    if (distance(x, y, i, j) < (5 * m_size / 6))
                        buffer_colour.set_colours(buffer[height][width], 255, 255, 255);
                    else
                        buffer_colour.set_colours(buffer[height][width], 100, 100, 100);
                }
                    

                
            }
        }
        for (int i = 0; i < m_size; ++i)
        {
            height = boundary_condition(y + i * sin(m_angle), SCREEN_HEIGHT);
            width = boundary_condition(x + i * cos(m_angle), SCREEN_WIDTH);
            buffer_colour.set_colours(buffer[height][width], 255, 0, 0);

            height = boundary_condition(y + 1 + i * sin(m_angle), SCREEN_HEIGHT);
            width = boundary_condition(x + i * cos(m_angle), SCREEN_WIDTH);
            buffer_colour.set_colours(buffer[height][width], 255, 0, 0);

            height = boundary_condition(y + i * sin(m_angle), SCREEN_HEIGHT);
            width = boundary_condition(x + 1 + i * cos(m_angle), SCREEN_WIDTH);
            buffer_colour.set_colours(buffer[height][width], 255, 0, 0);

            height = boundary_condition(y - 1 + i * sin(m_angle), SCREEN_HEIGHT);
            width = boundary_condition(x + i * cos(m_angle), SCREEN_WIDTH);
            buffer_colour.set_colours(buffer[height][width], 255, 0, 0);

            height = boundary_condition(y + i * sin(m_angle), SCREEN_HEIGHT);
            width = boundary_condition(x - 1 + i * cos(m_angle), SCREEN_WIDTH);
            buffer_colour.set_colours(buffer[height][width], 255, 0, 0);

        }
    }

    void turn(float dt, bool direction_left)
    {
        if (direction_left)
        {
            m_angle -= 2*dt;
            return;
        }
        m_angle += 2*dt;
    }

    void accelerate(float dt, bool direction_forward)
    {
        if (direction_forward)
        {
            m_speed += max_speed * dt / 2;
            if (m_speed > max_speed)
                m_speed = max_speed;
            return;
        }
        m_speed -= max_speed * dt;
        if (m_speed < 0)
            m_speed = 0;
    }

    void draw_healthbar()
    {
        int percent = (m_health / max_health) * 100;
        for (int i = 0; i < 100; ++i)
        {
            if (i < percent)
            {
                buffer_colour.set_colours(buffer[20][20 + i], 255, 0, 0);
                buffer_colour.set_colours(buffer[21][20 + i], 255, 0, 0);
            }
            else
            {
                buffer_colour.set_colours(buffer[20][20 + i], 0, 0, 255);
                buffer_colour.set_colours(buffer[21][20 + i], 0, 0, 255);
            }
        }
    }

} my_ship;

class Bullet : public Space_object
{
private:
    const int red = 255;
    const int green = 0;
    const int blue = 0;


public:
   Bullet(double x, double y, double angle, double speed = 400, int health = 1, int size = 0) 
       : Space_object(x, y, angle, speed, health, size) {}

   virtual void hit(double dt) override
   {
       m_health--;
   }

   virtual void move(float dt) override
   {
       m_x += m_speed * cos(m_angle) * dt;
       m_y += m_speed * sin(m_angle) * dt;
   }

   virtual void draw() override
   {
       int x = int(m_x);
       int y = int(m_y);
       if (x + 1 >= SCREEN_WIDTH || x - 1 < 0 || y + 1 > SCREEN_HEIGHT || y - 1 < 0)
       {
           m_health = 0;
           return;
       }
       buffer_colour.set_colours(buffer[y][x], red, green, blue);
       buffer_colour.set_colours(buffer[y + 1][x], red, green, blue);
       buffer_colour.set_colours(buffer[y - 1][x], red, green, blue);
       buffer_colour.set_colours(buffer[y][x + 1], red, green, blue);
       buffer_colour.set_colours(buffer[y][x - 1], red, green, blue);
   }
};

class Asteroid : public Space_object
{
private:
    int red;
    int green;
    int blue;

public:
    Asteroid(double x, double y, double angle, double speed = 100, double health = 10, int size = 15)
        : Space_object(x, y, angle, speed, health, size) 
    {
        switch (rand() % 3)
        {
            case 0:
                red = 150;
                green = 75;
                blue = 0;
                break;
            case 1:
                red = 128;
                green = 128;
                blue = 128;
                break;
            case 2:
                red = 64;
                green = 58;
                blue = 58;
                break;
        }
    }



    virtual void draw() override
    {
        int x = int(m_x);
        int y = int(m_y);
        int height;
        int width;
        for (auto i = x - m_size; i <= x + m_size; ++i)
        {
            for (auto j = y - m_size; j <= y + m_size; ++j)
            {
                if (distance(x, y, i, j) < m_size) 
                {
                    height = boundary_condition(j, SCREEN_HEIGHT);
                    width = boundary_condition(i, SCREEN_WIDTH);
                    buffer_colour.set_colours(buffer[height][width], red, green, blue);
                }
                
            }
        }
    }
};

std::vector <Space_object*> Space_objects;

bool generate_random_coordinates(int& x, int& y, double& angle, std::vector <Space_object*>& objects, int reserve = 250)
{
    x = reserve + rand() % (SCREEN_WIDTH - (2 * reserve));
    y = reserve + rand() % (SCREEN_HEIGHT - (2 * reserve));
    angle = rand() % 6;
    for (auto i = objects.begin(); i != objects.end(); ++i)
    {
        if (distance(x, y, (*i)->get_x(), (*i)->get_y()) < reserve)
            return false;
    }
    if (distance(x, y, my_ship.get_x(), my_ship.get_y()) < reserve)
        return false;
    return true;
}

// initialize game data in this function
void initialize()
{
    for (int i = 0; i < 10; ++i)
    {
        int asteroid_x = 0;
        int asteroid_y = 0;
        double asteroid_angle = 0;
        if (generate_random_coordinates(asteroid_x, asteroid_y, asteroid_angle, Space_objects))
        {
            Space_objects.push_back(new Asteroid(asteroid_x, asteroid_y, asteroid_angle));
        }
    }
}

// this function is called to update game data,
// dt - time elapsed since the previous update (in seconds)
void act(float dt)
{
    initialize();
    if (is_key_pressed(VK_LEFT))
        my_ship.turn(dt, true);
    if (is_key_pressed(VK_RIGHT))
        my_ship.turn(dt, false);
    if (is_key_pressed(VK_UP))
        my_ship.accelerate(dt, true);
    else
        my_ship.accelerate(dt, false);
    if (is_key_pressed(VK_SPACE))
    {
        Space_objects.push_back(new Bullet(my_ship.get_x(), my_ship.get_y(), my_ship.get_angle()));
    }
    std::vector <Space_object*> Space_objects_temp;
    for (auto i = Space_objects.begin(); i != Space_objects.end(); ++i)
    {
        (*i)->move(dt);
        for (auto j = i + 1; j != Space_objects.end(); ++j)
        {
            if (distance((*i)->get_x(), (*i)->get_y(), (*j)->get_x(), (*j)->get_y()) < ((*i)->get_size() + (*j)->get_size()))
            {
                (*i)->hit(dt);
                (*j)->hit(dt);
            }
        }
        if ((*i)->alive())
        {
            Space_objects_temp.push_back(*i);
        }  
        else
        {
            int new_size = (*i)->get_size() - 5;
            if (new_size > 0)
            {
                double psi = rand() % 6;
                Space_objects_temp.push_back(new Asteroid((*i)->get_x() + 10, (*i)->get_y() + 10, psi, (*i)->get_speed() * 1.5, 5, new_size));
                Space_objects_temp.push_back(new Asteroid((*i)->get_x() - 10, (*i)->get_y() - 10, psi + 3.14, (*i)->get_speed() * 1.5, 5, new_size));
            }
        }
    }
    Space_objects.clear();
    Space_objects.shrink_to_fit();
    my_ship.move(dt);
    for (auto i = Space_objects_temp.begin(); i != Space_objects_temp.end(); ++i)
    {
        Space_objects.push_back(*i);
    }
    for (auto i = Space_objects.begin(); i != Space_objects.end(); ++i)
    {
        if ((*i)->get_size() > 0 && distance((*i)->get_x(), (*i)->get_y(), my_ship.get_x(), my_ship.get_y()) < ((*i)->get_size() + my_ship.get_size()))
            my_ship.hit(dt);
    }
    if (is_key_pressed(VK_ESCAPE))
        schedule_quit_game();

  
}

// fill buffer in this function
// uint32_t buffer[SCREEN_HEIGHT][SCREEN_WIDTH] - is an array of 32-bit colors (8 bits per R, G, B)
void draw()
{
    
  // clear backbuffer
  memset(buffer, 0, SCREEN_HEIGHT * SCREEN_WIDTH * sizeof(uint32_t));
  for (auto i = Space_objects.begin(); i != Space_objects.end(); ++i)
      (*i)->draw();
  if (my_ship.alive()) 
  {
      my_ship.draw();
  }
  my_ship.draw_healthbar();

}

// free game data in this function
void finalize()
{
    for (auto i = Space_objects.begin(); i != Space_objects.end(); ++i)
    {
        delete (*i);
    }
    Space_objects.clear();
    Space_objects.shrink_to_fit();
}

