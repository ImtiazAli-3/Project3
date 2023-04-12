#include "pickup.h"
pickup::pickup(const engine::game_object_properties props) : engine::game_object(props)
{}
pickup::~pickup()
{}
void pickup::init()
{//pickup class to imitate game pickups
	//the pickup is rotated and does nothing else. i have another use for it
	m_is_active = true;
}
void pickup::update(glm::vec3 c, float dt)
{
	set_rotation_amount(rotation_amount() + dt * 1.5f); //rotates the pickup
	glm::vec3 d = position() - c;
	if (glm::length(d) < 2.f) {
		//m_is_active = false;
	}
}
engine::ref<pickup> pickup::create(const engine::game_object_properties& props)
{
	return std::make_shared<pickup>(props);
}
