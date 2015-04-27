
from gtsvm import *

    
def _debug_gen_switch_multi_class(x,dim):
    b=np.linalg.norm(np.ones(dim))
    xn=np.linalg.norm(x)
    if  xn > b*0.6 :
        return 2
    elif xn > b*0.3 :
        return 1
    else :
        return 0
    
    
def _debug():
    import os.path
    import numpy as np
    trainX_fname='mytrainX.npy'
    trainY_fname='mytrainY.npy'
    taindat_fname='mytrain.dat'
    if os.path.exists(trainX_fname) :
        X=np.load(trainX_fname)
        Y=np.load(trainY_fname)
        X=X.tolist()
        Y=Y.tolist()
    else:
        import random
        #N=20
        N=400  
        dim=4
        X=[[random.random() for m in range(dim)] for n in range(N)]
        ###Y=[random.choice([-1,1]) for n in range(N)]
        #Y=[1 if np.linalg.norm(x) > np.linalg.norm(np.ones(dim)*0.5) else -1 for x  in X] ##2class
        Y=[_debug_gen_switch_multi_class(x,dim) for x in X] ##muti class
        np.save(trainX_fname,X)
        np.save(trainY_fname,Y)
        dump_svmlight_file(X,Y,taindat_fname ,zero_based=False)

    print np.array(X)
    print Y

    #sys.exit()

    # print 'libsvm python ffi----------------------------'
    # # ### libsvm python ffi
    # import libsvm
    # import libsvm.svmutil
    # params = libsvm.svmutil.svm_parameter('-t 2 -c 2 -g 0.7')
    # print params
    # study_data = libsvm.svmutil.svm_problem(Y, X)    
    # model = libsvm.svmutil.svm_train(study_data, params)
    # # #model = libsvm.svmutil.svm_train(Y,X, params)
    # print model
    # Ypred_libsvmffi = libsvm.svmutil.svm_predict(Y,X , model)
    # print Ypred_libsvmffi
    # print map(int,Ypred_libsvmffi[0])
    
    ### sklearn
    print 'sklean -------------------'
    import numpy as np

    X = np.array(X)
    Y = np.array(Y)
    #print X
    #print Y

    # X = np.array([[-1, -1], [-2, -1], [1, 1], [2, 1]])
    # Y = np.array([1, 1, 2, 2])

    from sklearn.svm import SVC
    #clf = SVC()
    #SVC(C=3.0, cache_size=200, class_weight=None, coef0=1, degree=3, gamma=1, kernel='rbf', max_iter=-1, probability=False,random_state=None, shrinking=True, tol=0.001, verbose=False)
    clf=SVC(C=3.0, gamma=0.7, kernel='rbf')
    clf.fit(X, Y)
    #print(clf.predict([X[0]]))
    Ypred_sklearnSVC=clf.predict(X)
    print  Ypred_sklearnSVC.tolist()
    from sklearn.metrics import accuracy_score
    print accuracy_score(Y, Ypred_sklearnSVC)
    #print Y.tolist()
    
    print 'cmdline wrap-------------------'
    #clfcmd=ClassifyGtsvm(C=3.0, gamma=np.sqrt(0.7), kernel='rbf')
    clfcmd=ClassifyGtsvm(C=3.0, gamma=0.7*0.7, kernel='rbf')    
    clfcmd.fit(X, Y)
    Ypred_cmdwrap=clfcmd.predict(X)
    print Ypred_cmdwrap
    print Ypred_sklearnSVC.tolist()
    print Y.tolist()

        
    
#def cross_validation(X,Y):

##### grid.py
# class LocalWorker(Worker):
# 	def run_one(self,c,g):
# 		cmdline = self.get_cmd(c,g)
# 		result = Popen(cmdline,shell=True,stdout=PIPE,stderr=PIPE,stdin=PIPE).stdout
# 		for line in result.readlines():
# 			if str(line).find('Cross') != -1:
# 				return float(line.split()[-1][0:-1])

#####return string line example    
##Cross Validation Accuracy = 60%





if __name__ == '__main__':
    print 'main'
    _debug()
    #main()
    
    
