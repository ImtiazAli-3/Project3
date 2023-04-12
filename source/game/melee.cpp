#include "pch.h"
#include "melee.h"

melee::melee()
{}

melee::~melee()
{}
//melee class, similar to ballistic class in which a projectile is fired
void melee::initialise(engine::ref<engine::game_object> object)
{
	m_object = object;
}

void melee::fire(const engine::perspective_camera& camera, const glm::vec3& player_position, float speed)
{
	//object is fired downwards, a bounding box is used to simulate a large AOE melee attack
	m_object->set_position(player_position);
	m_object->set_forward(-camera.up_vector());
	m_speed = 2.0f;
}

void melee::on_update(const engine::timestep& time_step)
{
	m_object->set_position(m_object->position() + m_object->forward() * (float)time_step * m_speed);
}

void melee::on_render(const engine::ref<engine::shader>& shader)
{
	engine::renderer::submit(shader, m_object);
}
