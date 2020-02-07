#pragma once
#include <DxLib.h>
#include "DXLib_mat.hpp"
#include <string_view>
class VECTOR_ref {
	VECTOR value;

public:
	//����
	VECTOR_ref() noexcept : value(DxLib::VGet(0, 0, 0)) {}
	VECTOR_ref(VECTOR value) { this->value = value; }
	//���Z
	VECTOR_ref operator+(VECTOR_ref obj) {
		return VECTOR_ref(DxLib::VAdd(this->value, obj.value));
	}
	VECTOR_ref operator+=(VECTOR_ref obj) {
		this->value = DxLib::VAdd(this->value, obj.value);
		return this->value;
	}
	//���Z
	VECTOR_ref operator-(VECTOR_ref obj) const noexcept {
		return VECTOR_ref(DxLib::VSub(this->value, obj.value));
	}
	VECTOR_ref operator-=(VECTOR_ref obj){
		this->value = DxLib::VSub(this->value, obj.value);
		return this->value;
	}
	//�X�P�[�����O
	VECTOR_ref operator*(float p1) const noexcept {
		return VECTOR_ref(DxLib::VScale(this->value, p1)); 
	}
	VECTOR_ref operator*=(float p1){
		this->value = DxLib::VScale(this->value, p1);
		return this->value;
	}
	//�O��
	VECTOR_ref cross(VECTOR_ref obj) const noexcept {
		return VECTOR_ref(DxLib::VCross(this->value, obj.value));
	}
	//����
	float dot(VECTOR_ref obj) const noexcept {
		return DxLib::VDot(this->value, obj.value);
	}
	//�s��擾
	MATRIX_ref Mtrans() const noexcept { return DxLib::MGetTranslate(this->value); }
	//MATRIX MRotAxis(float p1) const noexcept { return DxLib::MGetRotAxis(this->value, p1); }
	//���K��
	VECTOR_ref Norm() const noexcept { return VECTOR_ref(DxLib::VNorm(this->value)); }
	//�T�C�Y
	float size() const noexcept { return DxLib::VSize(this->value); }
	//�o��
	VECTOR get() const noexcept { return this->value; }
	//���ꂼ��̒l
	float x() const noexcept { return this->value.x; }
	float x(float px) {
		this->value.x = px;
		return this->value.x;
	}
	float y() const noexcept { return this->value.y; }
	float y(float py) {
		this->value.y = py;
		return this->value.y;
	}
	float z() const noexcept { return this->value.z; }
	float z(float pz) {
		this->value.z = pz;
		return this->value.z;
	}
};
