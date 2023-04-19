#include <bits/stdc++.h>

using namespace std;

int main() {
	vector<int> v;
	for (int i = 0; i < 20; i++) {
		v.push_back(i);
	}
	for (int i = 0; i < v.size(); i++) {
		cout << i << ' ' << v[i] << '\n';
	}
	cout << '\n';
	for (int i = 0; i < v.size(); i++) {
		cout << i << ' ' << v[i] << '\n';
		v.erase(v.begin() + i--);
	}

}