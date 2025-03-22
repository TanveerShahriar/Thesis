#include <iostream>
#include "func.h"

using namespace std;

int x = 5;

int funcE(int a, int b){
    cout << "Inside E " << endl;
    x += 5;
    return 2 * a + 2 * b;
}

void funcC(){
    cout << "Start C " << endl;

    funcD(4, 5);

    cout << "End C " << endl;
}

void funcA(){
    cout << "Start A " << endl;

    funcB();

    cout << "End A " << endl;
}

int main(){
    funcA();
    return 0;
}