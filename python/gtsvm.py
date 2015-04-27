#!/usr/bin/env python

import uuid
import subprocess
import shlex
import csv
import sys
import time
from sklearn.base import BaseEstimator,RegressorMixin, ClassifierMixin
from sklearn.datasets import load_svmlight_file,dump_svmlight_file

import numpy as np
import scipy.sparse
import scipy

path_to_train_program='../bin/'


class ClassifyGtsvm(BaseEstimator, ClassifierMixin):


    def __init__(self
                 ,base_str='/tmp/'+uuid.uuid4().hex
                 ,C=1
                 ,kernel='rbf'
                 ,gamma=1
                 ,tol=0.001
                 ,max_iter=1000000
                 ):
        argdict= locals()
        argdict.pop('argdict',None)
        argdict.pop('self',None)
        vars(self).update(argdict)
        t_str=''
        # if kernel=='linear':
        #     t_str='-t 0'
        # if kernel=='poly':
        #     t_str='-k polynomial'        
        #     self.param_str=' {0} -C {1} -1 {2}'.format(t_str,C,gamma)
        # elif kernel=='sigmoid':
        #     t_str='-k sigmoid'
        # else: # kernel=='rbf':
        
        t_str='-k gaussian'
        self.param_str=' {0} -C {1} -1 {2}'.format(t_str,C,gamma)
            
        #self.param_str=' {0} -C {1} -1 {2}'.format(t_str,C,gamma)
        self.predict_fname=''
        self.test_fname=''
        self.train_fname=''
        self.model_fnaem=''
        

        
        
    def __del__(self):
        p = subprocess.Popen(['rm',self.model_fname ,self.train_fname , self.test_fname ,  self.predict_fname], stdout=None,stderr=None)
        p.wait()

        
    def fit(self, X, Y):

        self.labels=list(set(Y))
        if len(self.labels) > 2 :
            self.multiclass=True
            #print 'multiclass'
        else:
            self.multiclass=False
            
        self.train_fname =self.base_str +'-svmcmd-train' +  '.dat'
        self.model_fname =self.train_fname + '.model'
        dump_svmlight_file(X,Y,self.train_fname ,zero_based=False)
        if self.multiclass:
            command_line=path_to_train_program+'gtsvm_initialize {0} -f {1} -o {2}  -m 1 '.format(self.param_str, self.train_fname , self.model_fname )
        else:
            command_line=path_to_train_program+'gtsvm_initialize -f {1} -o {2} {0}'.format(self.param_str, self.train_fname , self.model_fname )
        args = shlex.split(command_line)
        p = subprocess.Popen(args)
        p.wait()
        command_line=path_to_train_program+'gtsvm_optimize -i {0} -o {1} -e {2} -n {3}'.format(self.model_fname,self.model_fname,self.tol,self.max_iter)    
        args = shlex.split(command_line)
        p = subprocess.Popen(args,stderr=subprocess.PIPE)
        p.wait()
        opt_err_str=p.stderr.read() ##gtsvm is too buggy
        if len(opt_err_str) < 1: 
            command_line=path_to_train_program+'gtsvm_shrink -i {0}  -o {1}'.format(self.model_fname,self.model_fname)
            args = shlex.split(command_line)
            p = subprocess.Popen(args)
            p.wait()
            self.train_fail=False
        else :
            self.train_fail=True
            
        
        return self
    
    def predict(self, X):
        if isinstance(X,list):
            self.test_n_sample=len(X)
        else:
            self.test_n_sample=X.shape[0]
        Y=[1]*self.test_n_sample
        self.test_fname =self.base_str +'-svmcmd-test' +  '.dat'
        self.predict_fname =self.base_str +'-svmcmd-predict' +  '.dat'
        dump_svmlight_file(X,Y,self.test_fname ,zero_based=False)
        command_line=path_to_train_program+'gtsvm_classify -f {0}  -i {1} -o {2}'.format(self.test_fname , self.model_fname, self.predict_fname )
        args = shlex.split(command_line)
        p = subprocess.Popen(args)
        p.wait()
        if self.train_fail:
            return [max(self.labels)+1]*self.test_n_sample
        
        if self.multiclass : 
            f = open(self.predict_fname, 'rb')
            self.predicted_weight = map(lambda row: map(float,row), list(csv.reader(f)))
            f.close()
            Y_predict=map(np.argmax, self.predicted_weight)
        else :
            self.predicted_weight = np.loadtxt( self.predict_fname)
            Y_predict=map(int,map(round,self.predicted_weight))
        return Y_predict


def cross(args):
    
    X,Y=load_svmlight_file(args.train_fname)
    from sklearn import cross_validation
    clfcmd=ClassifyGtsvm(C=args.c, gamma=args.g, kernel='rbf')
    scores = cross_validation.cross_val_score(clfcmd, X, Y, cv=args.v,n_jobs=1)
    time.sleep(1)
    print 'Cross Validation Accuracy = {0}%'.format(int(np.mean(scores)*100))


from argparse import ArgumentParser

def main():
    
    desc = u'{0} [Args] [Options]\n -h'.format(__file__)
    parser = ArgumentParser(description=desc)

    parser.add_argument('train_fname',  action='store',  nargs=None, const=None,  default=None, type=str, choices=None, help='train dat file', metavar=None)

    parser.add_argument('model_fname',  action='store',  nargs='?',  const=None,  default=None, type=str, choices=None, help='model file', metavar=None)

    parser.add_argument(
        '-v', 
        type = int,
        dest = 'v',
        #default = 5,
        help = 'cross '
    )

    parser.add_argument(
        '-c', 
        type = float,
        dest = 'c',
        default = 1,
        help = 'c'
    )

    parser.add_argument(
        '-g', 
        type = float,
        dest = 'g',
        #default = 5,
        help = 'g'
    )
    
    args = parser.parse_args()


    if args.v :
        cross(args)

    
if __name__ == '__main__':
    main()
    
    
    
