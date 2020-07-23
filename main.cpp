#include <iostream>
#include <string>
#include <random>
#include <array>
#include <vector>
#include <ctime>
#include "gnuplot-iostream.h"

using namespace std;

const double temp_max=100.;
const double temp_min=1.;
const double discount_factor=0.98;
const int swaps_per_temp=100000;

const int num_points=50;
const int points_range=1000;

int main()
{
    string path;
    cout << "Please enter path to Gnuplot: " << endl;
    getline(cin,path);
    Gnuplot gp("\""+path+"\""+" -persist");
    gp << "set xrange [0:1000]\nset yrange [0:1000]\n";
    random_device rd;
    seed_seq seed{std::time(0)};
    mt19937 rng(seed);

    uniform_int_distribution<int> unistart(0,points_range);
    uniform_int_distribution<int> uniswap(0,num_points-1);
    uniform_real_distribution<double> unitemp(0.,1.);

    array<array<int,2>,num_points> parent;

    for (int i=0;i<num_points;i++)
    {
        parent[i][0]=unistart(rng);
        parent[i][1]=unistart(rng);
    }

    //check for duplicate points.
    bool twice=false;
    while (true)
    {
        twice=false;
        for (int i=0;i<num_points;i++)
        {
            for (int j=i+1;j<num_points;j++)
            {
                if (parent[i][0]==parent[j][0] && parent[i][1]==parent[j][1])
                {
                    //same point!
                    twice=true;
                    parent[j][0]=unistart(rng);
                    parent[j][1]=unistart(rng);
                }
            }
        }
        if (!twice)
        {
            break;
        }
    }

    int step=0;
    double temp=temp_max*pow(discount_factor,step);
    double parent_dist=0.;
    double child_dist=0.;
    int tempx=0;
    int tempy=0;
    int a=0;
    int b=0;

    double best_dist=pow(10.,200.);
    vector<array<int,2> > best_one;
    array<int,2> temparray={};

    double prob=0.;

    while (temp>temp_min)
    {
        temp=temp_max*pow(discount_factor,step);
        cout << "Temp: " << temp << endl;

        for (int c=0;c<swaps_per_temp;c++)
        {
            //get the fitness of the parent.
            parent_dist=0.;
            for (int i=0;i<num_points-1;i++)
            {
                parent_dist+=sqrt(pow(parent[i][0]-parent[i+1][0],2.)+pow(parent[i][1]-parent[i+1][1],2.));
            }
            parent_dist+=sqrt(pow(parent[0][0]-parent[num_points-1][0],2.)+pow(parent[0][1]-parent[num_points-1][1],2.));

            //swap two points on the parent list.
            a=uniswap(rng);
            b=uniswap(rng);

            child_dist=parent_dist;
            child_dist-=sqrt(pow(parent[a][0]-parent[(a-1+num_points)%num_points][0],2.)+pow(parent[a][1]-parent[(a-1+num_points)%num_points][1],2.));
            child_dist-=sqrt(pow(parent[a][0]-parent[(a+1+num_points)%num_points][0],2.)+pow(parent[a][1]-parent[(a+1+num_points)%num_points][1],2.));
            child_dist-=sqrt(pow(parent[b][0]-parent[(b-1+num_points)%num_points][0],2.)+pow(parent[b][1]-parent[(b-1+num_points)%num_points][1],2.));
            child_dist-=sqrt(pow(parent[b][0]-parent[(b+1+num_points)%num_points][0],2.)+pow(parent[b][1]-parent[(b+1+num_points)%num_points][1],2.));

            child_dist+=sqrt(pow(parent[a][0]-parent[(b-1+num_points)%num_points][0],2.)+pow(parent[a][1]-parent[(b-1+num_points)%num_points][1],2.));
            child_dist+=sqrt(pow(parent[a][0]-parent[(b+1+num_points)%num_points][0],2.)+pow(parent[a][1]-parent[(b+1+num_points)%num_points][1],2.));
            child_dist+=sqrt(pow(parent[b][0]-parent[(a-1+num_points)%num_points][0],2.)+pow(parent[b][1]-parent[(a-1+num_points)%num_points][1],2.));
            child_dist+=sqrt(pow(parent[b][0]-parent[(a+1+num_points)%num_points][0],2.)+pow(parent[b][1]-parent[(a+1+num_points)%num_points][1],2.));

            if ((b-a+num_points)%num_points==1 || (b-a+num_points)%num_points==num_points-1)
            {
                child_dist+=2*sqrt(pow(parent[a][0]-parent[b][0],2.)+pow(parent[a][1]-parent[b][1],2.));
            }
            if (child_dist<parent_dist)
            {
                tempx=parent[a][0];
                tempy=parent[a][1];
                parent[a][0]=parent[b][0];
                parent[a][1]=parent[b][1];
                parent[b][0]=tempx;
                parent[b][1]=tempy;
            }
            else
            {
                prob=unitemp(rng);
                if (prob<exp((parent_dist-child_dist)/temp))
                {
                    tempx=parent[a][0];
                    tempy=parent[a][1];
                    parent[a][0]=parent[b][0];
                    parent[a][1]=parent[b][1];
                    parent[b][0]=tempx;
                    parent[b][1]=tempy;
                }
            }
        }
        if (parent_dist<best_dist)
        {
            best_dist=parent_dist;
            best_one.clear();
            for (int i=0;i<num_points;i++)
            {
                temparray[0]=parent[i][0];
                temparray[1]=parent[i][1];
                best_one.push_back(temparray);
            }
            temparray[0]=parent[0][0];
            temparray[1]=parent[0][1];
            best_one.push_back(temparray);
            gp << "plot '-' with lines title 'best_one'\n";
            gp.send1d(best_one);
        }

        step+=1;
    }

    return 0;
}
