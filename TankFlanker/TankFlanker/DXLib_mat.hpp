#pragma once
#include <DxLib.h>
#include "DXLib_vec.hpp"
class MATRIX_ref {
	MATRIX value;
public:
	MATRIX_ref() noexcept : value(DxLib::MGetIdent()) {}
	MATRIX_ref(MATRIX value) { this->value = value; }
	//���Z
	MATRIX_ref operator+(MATRIX_ref obj)  const noexcept { return MATRIX_ref(DxLib::MAdd(this->value, obj.value)); }
	MATRIX_ref operator+=(MATRIX_ref obj) noexcept {
		this->value = DxLib::MAdd(this->value, obj.value);
		return this->value;
	}
	//��Z
	MATRIX_ref operator*(MATRIX_ref obj) { return MATRIX_ref(DxLib::MMult(this->value, obj.value)); }
	MATRIX_ref operator*=(MATRIX_ref obj) noexcept {
		this->value = DxLib::MMult(this->value, obj.value);
		return this->value;
	}
	//�T�C�Y�ύX
	MATRIX_ref Scale(float p1) const noexcept { return MATRIX_ref(DxLib::MScale(this->value, p1)); }
	//�t
	MATRIX_ref Inverse() const noexcept { return MATRIX_ref(DxLib::MInverse(this->value)); }
	//�o��
	MATRIX get() const noexcept { return this->value; }
};
