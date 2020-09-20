#include <iostream>

class B1 {
	public :
		void f() {
			std::cout << "B1::f()\n";
		}

		virtual void vf () {
			std::cout << "B1::vf()\n";
		}
};

class D1 : public B1{
	public :
		void vf () {
			std::cout << "D1::vf()\n";
		}

/*		void f () {
			std::cout << "D1::f()\n";
		} */
};

int main (void) {
	D1 d;

	d.f();

	d.vf();
}
