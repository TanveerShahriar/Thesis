#include <iostream>

using namespace std;

int funcE(int a, int b){
    cout << "Inside E " << endl;
    return 2 * a + 2 * b;
}

int funcD(int a, int b){
    cout << "Start D " << endl;

    int res = funcE(2, 3);

    cout << "End D " << res << " " << endl;
    return res - a + b;
}

void funcC(){
    cout << "Start C " << endl;

    funcD(4, 5);

    cout << "End C " << endl;
}

void funcB(){
    cout << "Start B " << endl;

    funcC();

    cout << "End B " << endl;
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