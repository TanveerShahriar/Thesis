int funcD(int a, int b){
    cout << "Start D " << endl;

    int res = funcE(2, 3);
    
    cout << x << endl; 
    cout << "End D " << res << " " << endl;
    return res - a + b;
}

void funcB(){
    cout << "Start B " << endl;

    funcC();

    cout << "End B " << endl;
}