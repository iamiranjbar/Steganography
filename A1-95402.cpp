#include <iostream>
#include <algorithm>
#include <fstream>
#include <vector>
using namespace std;

typedef struct pixel{
int red;
int green;
int blue;
}pixel;

typedef struct block
{
	int x;
	int y;
	int color;//0:blue 1:green 2:red
	float var;
}block;

bool compareByVar(const block &a, const block &b)
{
    return a.var > b.var;
}

/*bool open_file(string file_name){
	ifstream img_file(file_name.c_str());
	if(!img_file) {
		cerr << "can't open file [" << file_name << "]\n";
	return false;
	}
	return true;
}*/

int main(){
	string command,file_name;
	vector <unsigned char> headers(54);
	int length,width,offset,block_length,block_width;
	cout<<"Please enter command:"<<endl;
	cin>>command; //main command
	if (command=="encrypt"){
		cout<<"Please enter file name:"<<endl;
		cin>>file_name;
		//open_file(file_name);//opening file
		ifstream img_file(file_name.c_str());
		if(!img_file) {
			cerr << "can't open file [" << file_name << "]\n";
		return false;
		}

		/*Getting information of file*/
		for (int i=0;i<54;i++){
		headers[i]=img_file.get();
		}
		length=headers[18]+headers[19]*256+headers[20]*256*256+headers[21]*256*256*256;
		width=headers[22]+headers[23]*256+headers[24]*256*256+headers[25]*256*256*256;
		//cout<<width<<"-"<<length<<endl;
		/*Reading pixels*/
		offset=headers[10]+headers[11]*256+headers[12]*256*256+headers[13]*256*256*256;
		vector <unsigned char> unknown_data(offset-54);
		for(int i=54;i<offset;i++){
			unknown_data[i-54]=img_file.get();
		}
		vector <vector<pixel> > point(length,vector<pixel>(width));
		for (int i=0;i<length;i++){
			for(int j=0;j<width;j++){
			point[i][j].blue=img_file.get();
			point[i][j].green=img_file.get();
			point[i][j].red=img_file.get();
			}
			for (int z=0;z<(4-((3*width)%4))%4;z++){
				img_file.get();
			}
		}

		/*devide picture to blocks*/
		int sum=0;
		float mean=0;
		block_length=length/8;
		block_width=width/8;
		vector <vector <block> > blocks(block_length,vector <block>(3*block_width));
		for(int i=0;i<block_length;i++){
			for (int j=0;j<3*block_width;j++){
				blocks[i][j].x=8*(j/3);
				blocks[i][j].y=8*i;
				blocks[i][j].color=j%3;

				/*calculate variances*/
				for (int k=0;k<8;k++){
					for (int q=0;q<8;q++){
						if ((j%3)==0){
							sum+=((point[(blocks[i][j].x)+k][(blocks[i][j].y)+q]).blue)-(((point[(blocks[i][j].x)+k][(blocks[i][j].y)+q]).blue)%2);
						}
						else if ((j%3)==1){
							sum+=((point[(blocks[i][j].x)+k][(blocks[i][j].y)+q]).green)-(((point[(blocks[i][j].x)+k][(blocks[i][j].y)+q]).green)%2);
						}
						else if ((j%3)==2){
							sum+=((point[(blocks[i][j].x)+k][(blocks[i][j].y)+q]).red)-(((point[(blocks[i][j].x)+k][(blocks[i][j].y)+q]).red)%2);
					}
				}
			mean=((float)sum)/64;
				blocks[i][j].var=0;
				for (int k=0;k<8;k++){
					for (int q=0;q<8;q++){
						if ((j%3)==0){
							blocks[i][j].var+=((float)(((point[(blocks[i][j].x)+k][(blocks[i][j].y)+q]).blue)-(((point[(blocks[i][j].x)+k][(blocks[i][j].y)+q]).blue)%2))-mean)
							*((float)(((point[(blocks[i][j].x)+k][(blocks[i][j].y)+q]).blue)-(((point[(blocks[i][j].x)+k][(blocks[i][j].y)+q]).blue)%2))-mean);
						}
						else if ((j%3)==1){
							blocks[i][j].var+=((float)(((point[(blocks[i][j].x)+k][(blocks[i][j].y)+q]).green)-(((point[(blocks[i][j].x)+k][(blocks[i][j].y)+q]).green)%2))-mean)
							*((float)(((point[(blocks[i][j].x)+k][(blocks[i][j].y)+q]).green)-(((point[(blocks[i][j].x)+k][(blocks[i][j].y)+q]).green)%2))-mean);
						}
						else if ((j%3)==2){
							blocks[i][j].var+=((float)(((point[(blocks[i][j].x)+k][(blocks[i][j].y)+q]).red)-(((point[(blocks[i][j].x)+k][(blocks[i][j].y)+q]).red)%2))-mean)
							*((float)(((point[(blocks[i][j].x)+k][(blocks[i][j].y)+q]).red)-(((point[(blocks[i][j].x)+k][(blocks[i][j].y)+q]).red)%2))-mean);
						}
					}
				}
			}
				sum=0;
				mean=0;
			}
		}

		/*sort variances*/
		vector <block> sorted_blocks(3*block_length*block_width);
		for(int i=0;i<block_length;i++){
			for (int j=0;j<3*block_width;j++){
					sorted_blocks[(i*block_width*3)+j]=blocks[i][j];
			}
		}
		sort(sorted_blocks.begin(),sorted_blocks.end(),compareByVar);
		/*for(int i=0;i<block_length;i++){
			for (int j=0;j<3*block_width;j++){
					cout<<sorted_blocks[(i*block_width*3)+j].var << ", ";
			}
		}*/
		/*get seed*/
		int seed;
		cout<<"Please enter your seed:"<<endl;
		cin>>seed;
		if (seed<1000||seed>999999){
			cout<<"Wrong seed!"<<endl;
			return 0;
		}
		srand(seed);

		/*get message*/
		string sentence;
		cout<<"Please enter your sentence:";
		cin.ignore();
		getline(cin,sentence);
		sentence.push_back('\n');
		if (sentence.size()>block_width*block_length*3){
			cout<<"your message is too long!"<<endl;
		}

		/*Hide message*/
		vector <int> sentencebit(8*sentence.size());
		for (int i=0;i<sentence.length();i++){
			sentencebit[8*i]=sentence[i]%2;
			sentencebit[(8*i)+1]=(sentence[i]/2)%2;
			sentencebit[(8*i)+2]=(sentence[i]/4)%2;
			sentencebit[(8*i)+3]=(sentence[i]/8)%2;
			sentencebit[(8*i)+4]=(sentence[i]/16)%2;
			sentencebit[(8*i)+5]=(sentence[i]/32)%2;
			sentencebit[(8*i)+6]=(sentence[i]/64)%2;
			sentencebit[(8*i)+7]=(sentence[i]/128)%2;
			//cout<<sentencebit[(8*i)+7]<<sentencebit[(8*i)+6]<<sentencebit[(8*i)+5]<<sentencebit[(8*i)+4]<<sentencebit[(8*i)+3]<<sentencebit[(8*i)+2]<<sentencebit[(8*i)+1]<<sentencebit[(8*i)]<<endl;
			//cout<<"______________________"<<endl;
		}
		int random;
		for (int i=0;i<sentencebit.size();i++){
			random=rand()%64;
			if ((sorted_blocks[i].color==0)&&sentencebit[i]==0){
				point[((sorted_blocks[i].x)+(random/8))][((sorted_blocks[i].y)+(random%8))].blue=(point[((sorted_blocks[i].x)+(random/8))][((sorted_blocks[i].y)+(random%8))].blue)&254;
			}
			if ((sorted_blocks[i].color==1)&&sentencebit[i]==0){
				point[((sorted_blocks[i].x)+(random/8))][((sorted_blocks[i].y)+(random%8))].green=(point[((sorted_blocks[i].x)+(random/8))][((sorted_blocks[i].y)+(random%8))].green)&254;
			}
			if ((sorted_blocks[i].color==2)&&sentencebit[i]==0){
				point[((sorted_blocks[i].x)+(random/8))][((sorted_blocks[i].y)+(random%8))].red=(point[((sorted_blocks[i].x)+(random/8))][((sorted_blocks[i].y)+(random%8))].red)&254;
			}
			if ((sorted_blocks[i].color==0)&&sentencebit[i]==1){
				point[((sorted_blocks[i].x)+(random/8))][((sorted_blocks[i].y)+(random%8))].blue=(point[((sorted_blocks[i].x)+(random/8))][((sorted_blocks[i].y)+(random%8))].blue)|1;
			}
			if ((sorted_blocks[i].color==1)&&sentencebit[i]==1){
				point[((sorted_blocks[i].x)+(random/8))][((sorted_blocks[i].y)+(random%8))].green=(point[((sorted_blocks[i].x)+(random/8))][((sorted_blocks[i].y)+(random%8))].green)|1;
			}
			if ((sorted_blocks[i].color==2)&&sentencebit[i]==1){
				point[((sorted_blocks[i].x)+(random/8) )][((sorted_blocks[i].y)+(random%8))].red=(point[((sorted_blocks[i].x)+(random/8))][((sorted_blocks[i].y)+(random%8))].red)|1;
			}
		}

		/*Save picture*/
		ofstream output("output.bmp");
		if (!output.is_open()){
			cerr << "can't open file [output.bmp]"<<'\n';
		return false;
		}
		for (int i=0;i<54;i++){
			output<<headers[i];
		}
		for(int i=54;i<offset;i++){
			output<<unknown_data[i-54];
		}
		for (int i=0;i<length;i++){
			for(int j=0;j<width;j++){
			output<<((unsigned char)point[i][j].blue);
			output<<((unsigned char)point[i][j].green);
			output<<((unsigned char)point[i][j].red);
			}
			for (int z=0;z<(4-((3*width)%4))%4;z++){
				output<<((unsigned char)0);
			}
		}
	//	output.close();
	}
	else if (command=="decrypt"){
		cout<<"Please enter file name:"<<endl;
		cin>>file_name;
		//open_file(file_name);//opening file
		ifstream img_file(file_name.c_str());
		if(!img_file) {
			cerr << "can't open file [" << file_name << "]\n";
		return false;
		}

		/*Getting information of file*/
		for (int i=0;i<54;i++){
		headers[i]=img_file.get();
		}
		length=headers[18]+headers[19]*256+headers[20]*256*256+headers[21]*256*256*256;
		width=headers[22]+headers[23]*256+headers[24]*256*256+headers[25]*256*256*256;
		//cout<<width<<"-"<<length<<endl;
		/*Reading pixels*/
		offset=headers[10]+headers[11]*256+headers[12]*256*256+headers[13]*256*256*256;
		vector <unsigned char> unknown_data(offset-54);
		for(int i=54;i<offset;i++){
			unknown_data[i-54]=img_file.get();
		}
		vector <vector<pixel> > point(length,vector<pixel>(width));
		for (int i=0;i<length;i++){
			for(int j=0;j<width;j++){
			point[i][j].blue=img_file.get();
			point[i][j].green=img_file.get();
			point[i][j].red=img_file.get();
			}
			for (int z=0;z<(4-((3*width)%4))%4;z++){
				img_file.get();
			}
		}

		/*devide picture to blocks*/
		int sum=0;
		float mean=0;
		block_length=length/8;
		block_width=width/8;
		vector <vector <block> > blocks(block_length,vector <block>(3*block_width));
		for(int i=0;i<block_length;i++){
			for (int j=0;j<3*block_width;j++){
				blocks[i][j].x=8*(j/3);
				blocks[i][j].y=8*i;
				blocks[i][j].color=j%3;

				/*calculate variances*/
				for (int k=0;k<8;k++){
					for (int q=0;q<8;q++){
						if ((j%3)==0){
							sum+=((point[(blocks[i][j].x)+k][(blocks[i][j].y)+q]).blue)-(((point[(blocks[i][j].x)+k][(blocks[i][j].y)+q]).blue)%2);
						}
						else if ((j%3)==1){
							sum+=((point[(blocks[i][j].x)+k][(blocks[i][j].y)+q]).green)-(((point[(blocks[i][j].x)+k][(blocks[i][j].y)+q]).green)%2);
						}
						else if ((j%3)==2){
							sum+=((point[(blocks[i][j].x)+k][(blocks[i][j].y)+q]).red)-(((point[(blocks[i][j].x)+k][(blocks[i][j].y)+q]).red)%2);
					}
				}
			mean=((float)sum)/64;
				blocks[i][j].var=0;
				for (int k=0;k<8;k++){
					for (int q=0;q<8;q++){
						if ((j%3)==0){
							blocks[i][j].var+=((float)(((point[(blocks[i][j].x)+k][(blocks[i][j].y)+q]).blue)-(((point[(blocks[i][j].x)+k][(blocks[i][j].y)+q]).blue)%2))-mean)
							*((float)(((point[(blocks[i][j].x)+k][(blocks[i][j].y)+q]).blue)-(((point[(blocks[i][j].x)+k][(blocks[i][j].y)+q]).blue)%2))-mean);
						}
						else if ((j%3)==1){
							blocks[i][j].var+=((float)(((point[(blocks[i][j].x)+k][(blocks[i][j].y)+q]).green)-(((point[(blocks[i][j].x)+k][(blocks[i][j].y)+q]).green)%2))-mean)
							*((float)(((point[(blocks[i][j].x)+k][(blocks[i][j].y)+q]).green)-(((point[(blocks[i][j].x)+k][(blocks[i][j].y)+q]).green)%2))-mean);
						}
						else if ((j%3)==2){
							blocks[i][j].var+=((float)(((point[(blocks[i][j].x)+k][(blocks[i][j].y)+q]).red)-(((point[(blocks[i][j].x)+k][(blocks[i][j].y)+q]).red)%2))-mean)
							*((float)(((point[(blocks[i][j].x)+k][(blocks[i][j].y)+q]).red)-(((point[(blocks[i][j].x)+k][(blocks[i][j].y)+q]).red)%2))-mean);
						}
					}
				}
			}
				sum=0;
				mean=0;
			}
		}

		/*sort variances*/
		vector <block> sorted_blocks(3*block_length*block_width);
		for(int i=0;i<block_length;i++){
			for (int j=0;j<3*block_width;j++){
					sorted_blocks[(i*block_width*3)+j]=blocks[i][j];
			}
		}
		sort(sorted_blocks.begin(),sorted_blocks.end(),compareByVar);
		/*for(int i=0;i<block_length;i++){
			for (int j=0;j<3*block_width;j++){
					cout<<sorted_blocks[(i*block_width*3)+j].var << ", ";
			}
		}*/
		/*get seed*/
		int seed;
		cout<<"Please enter your seed:"<<endl;
		cin>>seed;
		if (seed<1000||seed>999999){
			cout<<"Wrong seed!"<<endl;
			return 0;
		}
		srand(seed);

		/*find pixels*/
		//int count=0;
		int random;
		vector <int> sentencebit(8);
		vector <unsigned char>sentence(0);
		unsigned char cur_char;
		for (int i=0;i<(sorted_blocks.size()/8);i++){
			for (int j=0;j<8;j++){
				random=rand()%64;
				if (sorted_blocks[(8*i)+j].color==0){
					sentencebit[j]=((point[((sorted_blocks[8*i+j].x)+(random/8))][((sorted_blocks[8*i+j].y)+(random%8))].blue)%2);
				}
				if (sorted_blocks[(8*i)+j].color==1){
					sentencebit[j]=((point[((sorted_blocks[8*i+j].x)+(random/8))][((sorted_blocks[8*i+j].y)+(random%8))].green)%2);
				}
				if (sorted_blocks[(8*i)+j].color==2){
					sentencebit[j]=((point[((sorted_blocks[8*i+j].x)+(random/8))][((sorted_blocks[8*i+j].y)+(random%8))].red)%2);
				}
			}
			cur_char=sentencebit[0]+sentencebit[1]*2+sentencebit[2]*4+sentencebit[3]*8+sentencebit[4]*16+sentencebit[5]*32+sentencebit[6]*64+sentencebit[7]*128;
			sentence.push_back(cur_char);
			//count++;
			if (cur_char=='\n'){
				break;
			}
		}
		//cout<<count<<endl;
		/*show decoded message*/
		for (int i = 0; i < sentence.size(); ++i){
			cout<<sentence[i];
		}
	}
	else{
		cout<<"Wrong Command!"<<endl;
	}

	return 0;
}
