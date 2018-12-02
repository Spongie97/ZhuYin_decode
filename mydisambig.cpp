#include<stdio.h>
#include<stdlib.h>
#include<iostream>
#include<vector>
#include<string>

#include "Ngram.h"
#include "VocabMap.h"

using namespace std;

#ifndef MAXWORD
#define MAXWORD 256
#endif

static Vocab voc;

// Deal with Vocab_None case
VocabIndex getVocIndex(VocabString w){
	VocabIndex wid = voc.getIndex(w);
	if(wid == Vocab_None)
		return voc.getIndex(Vocab_Unknown);
	else return wid;
}
// Calculate bigram probability
double getBigramProb(Ngram& lm, VocabString w1, VocabString w2)
{
    VocabIndex wid1 = getVocIndex(w1);
    VocabIndex wid2 = getVocIndex(w2);

    VocabIndex context[] = { wid1, Vocab_None };
    return lm.wordProb( wid2, context);
}
// Viterbi Initialization
void init_viterbi(Ngram& lm, VocabMap& map, VocabString* words, int size[], LogP pro[][1024], VocabIndex BackTrack[][1024], VocabIndex Index[][1024], Vocab& vocZhuYin, Vocab& vocBig5){
	Prob p;
	VocabIndex w;
	VocabMapIter iter(map, vocZhuYin.getIndex(words[0]));

	for(int i = 0; iter.next(w, p); size[0]++, ++i){
		VocabIndex index = getVocIndex(vocBig5.getWord(w));
		VocabIndex empty[] = {Vocab_None};
		LogP curP = lm.wordProb(index, empty);

		if(curP == LogP_Zero) curP = -100;
		pro[0][i] = curP;
		Index[0][i] = w;
		BackTrack[0][i] = 0;
	}
}
// Calculate Viterbi
LogP Viterbi(Ngram& lm, VocabMap& map, VocabString* words, unsigned count, Vocab& vocZhuYin, Vocab& vocBig5){
	LogP pro[MAXWORD][1024] = {0.0};
	VocabIndex BackTrack[MAXWORD][1024];
	VocabIndex Index[MAXWORD][1024];
	int size[MAXWORD] = {0};

	// Viterbi initialization
	init_viterbi(lm, map, words, size, pro, BackTrack, Index, vocZhuYin, vocBig5);

	// Viterbi Algorithm 
	for(int s = 1; s < count; ++s){
		Prob p;
		VocabIndex w;
		VocabMapIter iter(map, vocZhuYin.getIndex(words[s]));
		
		for(int j = 0; iter.next(w, p); size[s]++, ++j){

			VocabString w2 = vocBig5.getWord(w);
			LogP maxP = -1.0/0.0, pre;
			
			for(int i = 0; i < size[s-1]; ++i){

				VocabString w1 = vocBig5.getWord(Index[s-1][i]);

				double curP = getBigramProb(lm, w1, w2);
				double unigramP = getBigramProb(lm, Vocab_Unknown, w2);
				
				if(curP == LogP_Zero && unigramP == LogP_Zero)
					curP = -100;

				curP += pro[s-1][i];		
		
				if(curP > maxP){
					maxP = curP;
					BackTrack[s][j] = i;
				}
			}
			pro[s][j] = maxP;
			Index[s][j] = w;
		}
	}

	// maximize probability
	LogP maxP = -1.0/0.0;
	int end = -1;
	for(int i = 0; i < size[count-1]; i++){
		LogP curP = pro[count-1][i];
		if(curP > maxP){
			maxP = curP;
			end = i;
		}
	}

	// BackTrack from end
	int path[MAXWORD];
	path[count-1] = end;
	for(int i = count-2; i >= 0; i--)
		path[i] = BackTrack[i+1][path[i+1]];

	for(int i = 0; i < count; i++){
		cout << vocBig5.getWord(Index[i][path[i]]);
		if(i == count-1)
			cout << endl;
		else
			cout << " ";
	}

	return maxP;
}



int main(int argc, char* argv[]){

	int order = atoi(argv[8]);
	Vocab vocZhuYin, vocBig5;

	Ngram lm(voc, order);
	VocabMap map(vocZhuYin, vocBig5);

	File mapfile(argv[4], "r");
	map.read(mapfile);
	mapfile.close();

	File lmFile(argv[6], "r");
	lm.read(lmFile);
	lmFile.close();

	File testdata(argv[2], "r");

	char* str = NULL;
	while(str = testdata.getline()){
		VocabString words[50000];
		unsigned count = Vocab::parseWords(str, &words[1], 50000);
		words[0] = "<s>";
		words[count+1] = "</s>";
		LogP MaxP = Viterbi(lm, map, words, count+2, vocZhuYin, vocBig5);
	}

	testdata.close();

	return 0;
}
