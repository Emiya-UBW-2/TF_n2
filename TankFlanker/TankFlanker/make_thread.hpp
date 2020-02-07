#pragma once
#ifndef INCLUDED_thd_h_
#define INCLUDED_thd_h_

#include "sub.hpp"
#include "useful.hpp"
#include <thread> 
#include <chrono>
#include <array> 
//����
struct input {
	std::array<bool, 19> get; /*�L�[�p(�ꎞ�Ď�)*/
	std::array<bool, 4> get2; /*�L�[�p(�펞�Ď�)*/
};
struct output {
	bool ends{ false }; /*�I���t���O*/

	int y = 0;
};
//60fps���ێ�����������Z(box2D����)
class ThreadClass {
private:
	std::thread thread_1;
	void calc(input& p_in, output& p_out);
public:
	ThreadClass();
	~ThreadClass();
	void thread_start(input& p_in, output& p_out);
	void thead_stop();
};

#endif 