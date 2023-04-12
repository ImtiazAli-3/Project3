#include "pch.h"
#include "Ballistic.h"

ballistic::ballistic()
{}

ballistic::~ballistic()
{}
//ballistic class for the gun
void ballistic::initialise(engine::ref<engine::game_object> object)
{
	m_object = object;
}
//ballistic class, when a key is pressed the object fires from the camera position
void ballistic::fire(const engine::perspective_camera& camera, const glm::vec3& player_position, float speed)
{
	//sets the position of the projectile to where the player will be
	m_object->set_position(player_position);
	//the projectile travels across a path, the bottom equations allows it to follow from the front of the player
	m_object->set_forward(camera.front_vector()+camera.up_vector()/7.f);
	//m_object->set_forward(-camera.up_vector());
	m_speed = 6.0f;//speed of projectile
}

void ballistic::on_update(const engine::timestep& time_step)
{
	m_object->set_position(m_object->position() + m_object->forward() * (float)time_step * m_speed);
}

void ballistic::on_render(const engine::ref<engine::shader>& shader)
{
	engine::renderer::submit(shader, m_object);
}
