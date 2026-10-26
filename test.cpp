
#include<iostream>
#include<algorithm>
#include<vector>
using namespace std;

long long sum;  //总消耗体力，会大于int
int main() {
    int n;
    cin >> n;
    vector<int>a(305, 0);  //高度
    for (int i = 1; i <= n; i++) {
       a[i]=10;
       cin>>a[i];
    }

    //先对长度进行排序
    sort(a.begin()+1, a.begin()+1+n);
    //cout << *(a.begin() + 1) << " " << *(a.begin()+ n);
    //双指针
    int l = 0, r = n;  //第一次从地面开始
    while (l < r) {  //经检验，不加等号
        sum += (a[r] - a[l]) * (a[r] - a[l]);
        l++;

        sum += (a[r] - a[l]) * (a[r] - a[l]);
        r--;
    }
    cout << sum;
}

