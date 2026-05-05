#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
using namespace std;

// 核心：比较 a 和 b 谁放前面更大
bool compare(int a, int b) {
    string sa = to_string(a);
    string sb = to_string(b);
    // 关键规则：拼接后 "ab" > "ba" 则 a 在前
    return sa + sb > sb + sa;
}

// 输出排序后的结果
void printvec(vector<int>& temp) {
    sort(temp.begin(), temp.end(), compare);
    for (int num : temp) {
        cout << num;
    }
    cout << endl;
}

int main() {
    // 本地测试打开文件，提交注释掉
    // freopen("big.in", "r", stdin);
    
    string pstr;
    while (getline(cin, pstr)) {
        vector<int> bvec;
        string num;
        
        // 安全分割字符串（不破坏原字符串）
        for (char ch : pstr) {
            if (ch == ' ') {
                if (!num.empty()) {
                    bvec.push_back(stoi(num));
                    num.clear();
                }
            } else {
                num += ch;
            }
        }
        // 最后一个数字
        if (!num.empty()) {
            bvec.push_back(stoi(num));
        }
        
        printvec(bvec);
    }
    return 0;
}