#include <algorithm>
#include <vector>
#include <iostream>
#include <cstdio>
#include <cstring>
using namespace std;

#define LOCAL 1
#define Filename "LinkTree.in"

// 链表节点定义
struct ListNode {
    int val;
    ListNode *next;
    ListNode() : val(0), next(nullptr) {}
    ListNode(int x) : val(x), next(nullptr) {}
    ListNode(int x, ListNode *next) : val(x, next) {}
};

// ===================== 核心：两数相加 =====================
ListNode* addTwoNumbers(ListNode* l1, ListNode* l2) {
    ListNode dummy(0);  // 哨兵头节点，不用处理空指针
    ListNode* cur = &dummy;
    int carry = 0;  // 进位
    
    // 两个链表都走完 && 没有进位 才结束
    while (l1 || l2 || carry) {
        int sum = carry;
        
        if (l1) {
            sum += l1->val;
            l1 = l1->next;
        }
        if (l2) {
            sum += l2->val;
            l2 = l2->next;
        }
        
        carry = sum / 10;        // 新的进位
        cur->next = new ListNode(sum % 10);  // 新节点
        cur = cur->next;
    }
    return dummy.next;
}

// ===================== 遍历打印链表 =====================
void traverse(ListNode *p) {
    while (p) {
        cout << p->val;
        if (p->next) cout << ",";
        p = p->next;
    }
    cout << endl;
}

// ===================== 根据字符串构建链表 =====================
ListNode* buildList(string s) {
    ListNode dummy(0);
    ListNode* cur = &dummy;
    for (char c : s) {
        if (c == ',') continue;
        cur->next = new ListNode(c - '0');
        cur = cur->next;
    }
    return dummy.next;
}

// ===================== 主函数 =====================
int main() {
#ifdef LOCAL
    freopen(Filename, "r", stdin);
#endif

    string a, b;
    // 读取两行：第一行是 l1，第二行是 l2
    // 输入示例：
    // 2,4,3
    // 5,6,4
    cin >> a >> b;

    ListNode* l1 = buildList(a);
    ListNode* l2 = buildList(b);

    ListNode* res = addTwoNumbers(l1, l2);

    cout << "链表1："; traverse(l1);
    cout << "链表2："; traverse(l2);
    cout << "相加结果："; traverse(res);

    return 0;
}