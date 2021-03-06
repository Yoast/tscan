#!/usr/bin/python
# -*- coding: utf-8 -*-

import os, numpy, sys
from gensim import corpora, models, similarities, matutils
from twisted.internet import protocol, reactor #No Python 3 support yet :(
from twisted.protocols import basic

# het queryen van zinnen/teksten staat uitgelegd op http://radimrehurek.com/gensim/tut3.html .
# Om een of andere reden werkt het bij mij niet om losse woorden op deze manier met elkaar te vergelijken (documenten van 1 woord).
# Onderstaande methode waarbij "zelf" de similarity scores uitreken levert wel resultaten op die ok zijn.



class LSAProtocol(basic.LineReceiver):
    def lineReceived(self, line):
        # print( "LSA received " + line );
        try:
            word1, word2 = line.strip().split("\t")
        except:
            self.sendLine("INPUTERROR")
            return

        # print( "LSA words: " + word1 + "-" + word2 );
        try:
            index1 = word_id_dict[word1]
            index2 = word_id_dict[word2]
        except KeyError:
            self.sendLine(str(0))
            return
        # print( "LSA indices: " + str(index1) + "-" + str(index2) );
        try:
            cossim = numpy.dot(matutils.unitvec(US[index1, :]),matutils.unitvec(US[index2, :]))
        except:
            cossim = 0
        self.sendLine(str(cossim))

class LSAFactory(protocol.ServerFactory):
    def __init__(self, word_id_dict, US):
        self.word_id_dict = word_id_dict
        self.US = US

class LSAServer(object):
    def __init__(self, word_id_dict, US, port=65430):
        factory = LSAFactory(word_id_dict, US)
        factory.protocol = LSAProtocol;
        reactor.listenTCP(port, factory )
        reactor.run()

if __name__ == '__main__':
    try:
        port = int(sys.argv[1])
        dict_file, lsa_file = sys.argv[2:]
    except:
        print >>sys.stderr,"Usage: port dict_file lsa_file"
        sys.exit(2)

    print( "reading the data files. (takes some time...)" );
    #dict_file = 'newspaper_contentlemma_dictionary_filtered.dict'
    #bow_corp_file = 'newspaper_contentlemma_corpus_as_vectors_filtered.mm' //not used?
    #lsa_file = 'lsa_newspaper_lemma_model_700_filtered_tfidf.lsi'

    # Inladen van eerder gecreëerde corpus, dictionary en lsa objecten
    #corpus = corpora.MmCorpus(bow_corp_file) # I don't see the need for this one?
    lsi = models.LsiModel.load(lsa_file)
    dictionary = corpora.Dictionary.load(dict_file)

    # ik wil op woorden kunnen zoeken in een dictionary om de woord ID op te vragen, dus we moeten key,values omdraaien
    word_id_dict = {}
    for k,v in dictionary.iteritems():
        word_id_dict[v] = k

    U = lsi.projection.u
    S = lsi.projection.s

    US=numpy.multiply(U,S)
    print( "starting the server!" );
    LSAServer(word_id_dict, US, port)

