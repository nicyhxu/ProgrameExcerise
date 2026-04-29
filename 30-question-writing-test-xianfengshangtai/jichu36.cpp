#include<cstdio>
#include<algorithm>
#include<cstring>
#define CLR(x) memset(x,0,sizeof(x))
#define ll long long int
#define PI acos(-1.0)
#define db double
#define mod 1000000007
using namespace std;
const int maxn=1e5+5;
const db eps=1e-6;
const int inf=1e9;
const ll INF=1e15;
int c[maxn];         //树状数组
int lisan[maxn];   //用来离散的存离散后的结果数组
int s[maxn];        //用来存原始状态
int n,k;
//树状数组
int lowbit(int x)  //返回值最大1e5.
{
    return x&(-x);
}

void update(int i,int ans)
{
    while(i <= n){
        c[i] += ans;
        i += lowbit(i);
    }
}

int sum(int i)
{
    int s = 0;
    while(i > 0){
        s += c[i];
        i -= lowbit(i);
    }
    return s;
}
//结束
int main()
{
    while(~scanf("%d%d",&n,&k)){
        CLR(c);
        for(int i=0;i<n;i++){
            scanf("%d",&s[i]);
            lisan[i] = s[i];
        }
        sort(lisan,lisan+n);
        int len=unique(lisan,lisan+n)-lisan;
        ll res = 0;
        for(int i=0;i<n;i++){
            ll l=lower_bound(lisan,lisan+len,s[i])-lisan+1;
            update(l,1);
            res += i+1 - sum(l);
        }
        if(res < k) printf("0\n");
        else
          printf("%lld\n",res-k);
    }
}
