#pragma once
#include "Math.h"

struct Camera {
	Camera(Vec3 pos_, Vec3 target, float aspect_, float fov_ = 60.0f, Vec3 updir_ = Vec3(0, 1, 0), float near_z_ = 0.1, float far_z_ = 10000)
		:pos(pos_), dPos(pos),target(target), updir(updir_),
		fov(fov_), aspect(aspect_), near_z(near_z_), far_z(far_z_),
		dPitch(0), dYaw(0)
	{
		viewMat= Mat4::LookAt(pos, target, updir);
		perspectMat = Perspect();
	};

	//view
	Vec3 pos;
	Vec3 dPos;
	Vec3 target;	
	Vec3 updir;	//y

	Mat4 viewMat;

	//projection
	float fov, aspect, near_z, far_z;
	Mat4 perspectMat;
	float dPitch, dYaw;
	//camera control
	float sensitivity = 1.0f;
	
	const Mat4& View() const {
		return viewMat;
	}

	const Mat4 Perspect() const {
		return Mat4::Perspect(fov, aspect, near_z, far_z);
	}

	const Mat4& Ortho() const {
		return Mat4::Ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 10.0f);
	}

	void UpdateCameraPos(float x_offset,float y_offset) {
		x_offset *= sensitivity*PI;
		y_offset *= sensitivity*PI;
		Vec3 fordir = pos - target;
		float r = fordir.norm();

		float pitch = (float)acos(fordir.y / r);
		float yaw = (float)atan2(fordir.x, fordir.z);

		dPitch-= y_offset;
		dYaw -= x_offset;

		pitch += dPitch;
		yaw += dYaw;

		if (pitch > radians(180.0f)) {
			pitch = radians(179.0f);
		}
		if (pitch < 0) {
			pitch =radians(1.0f);
		}

		dPos.x = target.x + r * sin(pitch) * sin(yaw);
		dPos.y = target.y + r * cos(pitch);
		dPos.z = target.z + r * sin(pitch) * cos(yaw);


		Vec3 zn_axis = target- dPos;
		Vec3 x_axis = zn_axis.cross(updir);
		Vec3 y_axis = x_axis.cross(zn_axis);

		viewMat = Mat4::LookAt(dPos, target, y_axis.normalized());
	}


	void UpdateCameraFov(float y_offset) {
		fov += y_offset;
		if (fov < 1.0) {
			fov = 1.0;
		}else if (fov > 60.0) {
			fov = 60.0;
		}
		perspectMat = Perspect();
	}
};