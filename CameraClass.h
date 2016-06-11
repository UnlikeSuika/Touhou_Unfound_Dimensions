#ifndef _CAMERA_CLASS_H_
#define _CAMERA_CLASS_H_

#include <directxmath.h>

using namespace DirectX;

class CameraClass{
public:
	CameraClass();
	CameraClass(const CameraClass& other);
	~CameraClass();

	void SetPosition(float x, float y, float z);
	void SetRotation(float, float, float);
	XMFLOAT3 GetPosition();
	XMFLOAT3 GetRotation();
	void Render();
	void GetViewMatrix(XMMATRIX& viewMatrix);
private:
	float m_positionX;
	float m_positionY;
	float m_positionZ;
	float m_rotationX;
	float m_rotationY;
	float m_rotationZ;
	XMMATRIX m_viewMatrix;
};

#endif