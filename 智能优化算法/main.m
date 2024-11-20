%%
close all
clear 
tic % start to record the computation time
clc
SearchAgents_no=30; 
Max_iteration=1000; 
lb=-100;             
ub=100;              
dim=10; 
Runs_times=3;
Obtained_Best_fitness=zeros(12,Runs_times);
Min_Best_fitness=zeros(12,1);
Min_Convergence_curve=zeros(12,Max_iteration);
Min_Best_X= zeros(dim,1);
for F2=1:12
       Min=inf;
       clear Best_X Best_Cost Convergence_curve
       for F1=1:Runs_times
         fobj = @(x) cec22_test_func(x',F2);
          [Convergence_curve,Best_Cost,Best_X]=WAA(SearchAgents_no,Max_iteration,lb,ub,dim,fobj);
          
         Obtained_Best_fitness(F2,F1)=Best_Cost;
         if  Min>Best_Cost
             Min=Best_Cost;
			 Min_Best_fitness(F2,1)=Best_Cost;
             Min_Best_X(F2,1:length(Best_X))=Best_X;   
			 Min_Convergence_curve(F2,1:length(Convergence_curve))=Convergence_curve';
 		 end 
 end
end
elapsedTime = toc; % record and save the running time in the variant elapsedTime
disp(['Computational running time: ', num2str(elapsedTime), ' second']);
