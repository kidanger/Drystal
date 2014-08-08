/**
 * This file is part of Drystal.
 *
 * Drystal is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Drystal is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Drystal.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "module.hpp"
#include "world_bind.hpp"
#include "joint_bind.hpp"
#include "shape_bind.hpp"
#include "body_bind.hpp"
#include "api.hpp"

BEGIN_MODULE(physics)
	DECLARE_FUNCTION(init_physics)

	DECLARE_FUNCTION(set_gravity)
	DECLARE_FUNCTION(get_gravity)
	DECLARE_FUNCTION(set_pixels_per_meter)
	DECLARE_FUNCTION(get_pixels_per_meter)

	DECLARE_FUNCTION(new_shape)
	DECLARE_FUNCTION(new_body)
	DECLARE_FUNCTION(new_joint)

	DECLARE_FUNCTION(update_physics)
	DECLARE_FUNCTION(on_collision)

	DECLARE_FUNCTION(raycast)
	DECLARE_FUNCTION(query)

	BEGIN_CLASS(body)
		ADD_GETSET(body, position)
		ADD_METHOD(body, get_center_position)
		ADD_GETSET(body, angle)
		ADD_GETSET(body, linear_velocity)
		ADD_GETSET(body, angular_velocity)
		ADD_GETSET(body, linear_damping)
		ADD_GETSET(body, angular_damping)
		ADD_GETSET(body, fixed_rotation)

		ADD_METHOD(body, set_active)
		ADD_METHOD(body, set_bullet)
		ADD_METHOD(body, get_mass)
		ADD_METHOD(body, set_mass_center)
		ADD_METHOD(body, apply_force)
		ADD_METHOD(body, apply_linear_impulse)
		ADD_METHOD(body, apply_angular_impulse)
		ADD_METHOD(body, apply_torque)
		ADD_METHOD(body, dump)
		ADD_METHOD(body, destroy)
		ADD_GC(free_body)
	REGISTER_CLASS(body, "Body")

	BEGIN_CLASS(shape)
		ADD_GETSET(shape, density)
		ADD_GETSET(shape, restitution)
		ADD_GETSET(shape, friction)
		ADD_METHOD(shape, set_sensor)
		ADD_GC(gc_shape)
	REGISTER_CLASS(shape, "Shape")

	BEGIN_CLASS(mouse_joint)
		ADD_METHOD(joint, destroy)
		ADD_METHOD(mouse_joint, set_target)
		ADD_GC(free_joint)
	REGISTER_CLASS(mouse_joint, "MouseJoint")

	BEGIN_CLASS(distance_joint)
		ADD_METHOD(joint, destroy)
		ADD_METHOD(distance_joint, set_length)
		ADD_METHOD(distance_joint, set_frequency)
		ADD_GC(free_joint)
	REGISTER_CLASS(distance_joint, "DistanceJoint")

	BEGIN_CLASS(rope_joint)
		ADD_METHOD(joint, destroy)
		ADD_METHOD(rope_joint, set_max_length)
		ADD_GC(free_joint)
	REGISTER_CLASS(rope_joint, "RopeJoint")

	BEGIN_CLASS(revolute_joint)
		ADD_METHOD(joint, destroy)
		ADD_METHOD(revolute_joint, set_angle_limits)
		ADD_METHOD(revolute_joint, set_motor_speed)
		ADD_GC(free_joint)
	REGISTER_CLASS(revolute_joint, "RevoluteJoint")

	BEGIN_CLASS(friction_joint)
		ADD_METHOD(joint, destroy)
		ADD_METHOD(friction_joint, get_max_force)
		ADD_METHOD(friction_joint, get_max_torque)
		ADD_METHOD(friction_joint, set_max_force)
		ADD_METHOD(friction_joint, set_max_torque)
		ADD_GC(free_joint)
	REGISTER_CLASS(friction_joint, "FrictionJoint")

	BEGIN_CLASS(prismatic_joint)
		ADD_METHOD(joint, destroy)
		ADD_METHOD(prismatic_joint, set_enable_motor)
		ADD_METHOD(prismatic_joint, set_motor_speed)
		ADD_METHOD(prismatic_joint, set_enable_limit)
		ADD_METHOD(prismatic_joint, is_motor_enabled)
		ADD_METHOD(prismatic_joint, is_limit_enabled)
		ADD_GC(free_joint)
	REGISTER_CLASS(prismatic_joint, "PrismaticJoint")
END_MODULE()

