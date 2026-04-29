#include <iostream>
#include <cstdio>
#include <sstream>
#include <vector>
#include <queue>
using namespace std;
typedef struct task_job
{
	int ptime;
	int cost;
	int ltime;
}TASK_JOB;

typedef struct action
{
	TASK_JOB job;
	int used;
}ACTOIN;

int main()
{
    //freopen("1.in","r",stdin);
    string task;
	string job;
	int length,man;
	int a,b;
	getline(cin,task);
	getline(cin,job);
	stringstream astr(task);
	stringstream job_b(job);
	queue<TASK_JOB>queue_job;
	queue<TASK_JOB>queue_job_brother;
	queue<TASK_JOB>empty_queue;
	vector<ACTOIN>paction;
	while(astr>>a>>b)
	{
		TASK_JOB temp;
		temp.ptime =a;
		temp.cost = b;
		temp.ltime = a+b;
		queue_job.push(temp);
	}
	job_b>>length>>man;
	int time = 0;
	int drop = 0;
	for(int i=0;i<man;i++)
	{
		ACTOIN tmp;
		memset(&tmp,0,sizeof(ACTOIN));
		paction.push_back(tmp);
	}
	cout<<paction.size()<<endl;
	while(!queue_job.empty())
	{
		int asize = queue_job.size();
		for(int i=0;i<asize;i++)
		{
			TASK_JOB temp = queue_job.front();
			if(temp.ptime > time)
			{
				queue_job_brother.push(temp);
				cout<<"brother job"<<temp.ptime<<"current time"<<time<<endl;
			} else if(temp.ptime = time){
				bool find = 0;
				for(int i=0;i<man;i++)
				{
					if(paction[i].used == 0 && length > 0)
					{
						paction[i].used = 1;
						paction[i].job = temp;
						find = 1;
						length--;
                        cout<<"find job1"<<temp.ptime<<"current time"<<time<<endl;						
						break;
					}
					if(paction[i].used == 1 && paction[i].job.ltime <= time && length > 0)
					{
						paction[i].used = 1;
						paction[i].job = temp;
						find = 1;
						length--; 
						cout<<"find job2"<<temp.ptime<<"current time"<<time<<endl;			
						break;
					}
				}
				if(find==0) 
				{
					cout<<"drop 1 job ptime"<<temp.ptime<<"ltime"<<temp.ltime<<endl;
					drop++;
				}
			} else{
				cout<<"drop 2 job ptime"<<temp.ptime<<"ltime"<<temp.ltime<<endl;
				drop++;
			}
			queue_job.pop();
		}
		for(int i=0;i<man;i++)
		{
			if(paction[i].used == 1 && paction[i].job.ltime <= time)
			{
				paction[i].used = 0;
			}
		}
		time++;
		queue_job = queue_job_brother;
		queue_job_brother = empty_queue
		cout<<"queue_job size"<<queue_job.size()<<"queue_job_brother size"<<queue_job_brother.size()<<endl;
	}
    return 0;
}