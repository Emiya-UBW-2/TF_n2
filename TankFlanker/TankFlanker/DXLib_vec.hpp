#pragma once
#include <DxLib.h>
#include"DXLib_mat.hpp"
class VECTOR_ref {
	VECTOR value;
public:
	VECTOR_ref() noexcept : value(DxLib::VGet(0, 0, 0)) {}
	VECTOR_ref(VECTOR value) { this->value = value; }
	//���Z
	VECTOR_ref operator+(VECTOR_ref obj) { return VECTOR_ref(DxLib::VAdd(this->value, obj.value)); }
	VECTOR_ref operator+=(VECTOR_ref obj) {
		this->value = DxLib::VAdd(this->value, obj.value);
		return this->value;
	}
	//���Z
	VECTOR_ref operator-(VECTOR_ref obj) { return VECTOR_ref(DxLib::VSub(this->value, obj.value)); }
	VECTOR_ref operator-=(VECTOR_ref obj) {
		this->value = DxLib::VSub(this->value, obj.value);
		return VECTOR_ref(DxLib::VSub(this->value, obj.value));
	}
	//�O��
	VECTOR_ref operator*(VECTOR_ref obj) { return VECTOR_ref(DxLib::VCross(this->value, obj.value)); }
	//����
	float operator%(VECTOR_ref obj) { return DxLib::VDot(this->value, obj.value); }
	//�s��擾
	MATRIX_ref Mtrans() const noexcept { return DxLib::MGetTranslate(this->value); }
	//�T�C�Y�ύX
	VECTOR_ref Scale(float p1) const noexcept { return VECTOR_ref(DxLib::VScale(this->value, p1)); }
	//���K��
	VECTOR_ref Norm() const noexcept { return VECTOR_ref(DxLib::VNorm(this->value)); }
	//�T�C�Y
	float size() const noexcept { return DxLib::VSize(this->value); }
	//�o��
	VECTOR get() const noexcept { return this->value; }
	float x() const noexcept { return this->value.x; }
	float y() const noexcept { return this->value.y; }
	float z() const noexcept { return this->value.z; }

	float x(float p) noexcept { this->value.x = p; return this->value.x; }
	float y(float p) noexcept { this->value.y = p; return this->value.y; }
	float z(float p) noexcept { this->value.z = p; return this->value.z; }

	float xadd(float p) noexcept { this->value.x += p; return this->value.x; }
	float yadd(float p) noexcept { this->value.y += p; return this->value.y; }
	float zadd(float p) noexcept { this->value.z += p; return this->value.z; }
};
