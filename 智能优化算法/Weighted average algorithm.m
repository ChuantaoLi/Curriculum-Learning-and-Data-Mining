function [Convergence_curve,Best_Cost,Best_X, Average_curve]=WAA(nP,Max_It,lb,ub,dim,fobj)
%% Initialization
Convergence_curve=zeros(Max_It,1);
X = zeros(nP,dim);
pBest_X = zeros(nP,dim);  % Personal Best Position
Cost = zeros(nP,1);
pBest_Cost = zeros(nP,1);
for i=1:nP
    X(i,:)=unifrnd(lb,ub,1,dim);
    pBest_X(i,:) = X(i,:);
    Cost(i) = fobj(X(i,:));
    pBest_Cost(i) = Cost(i);
    
end
[Best_Cost,ind] = min(Cost);
Best_X = X(ind,:);
%% Main Loop
for it=1:Max_It  
    
    [~, I] = sort(Cost);
    for j=1:nP
	    X(j,:)=X(I(j),:);Cost(j,:)=Cost(I(j),:);
    end
 	miu=zeros(1,dim);
    sum_Cost=0;
	n =(nP-4)*(it-1)/(1-Max_It)+nP;
	for i=1:n
         sum_Cost=sum_Cost+Cost(i); 
	end
	
	for i=1:n
		miu(1,:)=miu(1,:)+X(i,:)*(sum_Cost-Cost(i))/(sum_Cost*(n-1));  
    end
			
    for i=1:nP
        k1= (10*rand-1)*sin(pi*it/Max_It );
        k2= randi([1,3],1,1);
        k3= rand;
        if  k1<0.5 
             if k3>0.5
               for j=1:dim
                   beta=1.5;
		           sigma1=gamma(1+beta)*sin(pi*beta/2)/(beta*gamma(0.5+0.5*beta)*2^(0.5*beta-0.5));
		           levy(j)=normrnd(0,sigma1^2)/abs(normrnd(0,1))^(-beta);
		            X(i,j)= Best_X(1,j)+levy(j);
               end
             else 
                X(i,:)=lb+rand*(ub-lb);
             end
        else
           switch k2
              case 1
                  X(i,:) =rand*(miu(1,:)-Best_X(1,:))+rand*(miu(1,:)-pBest_X(i,:))+rand*miu(1,:);               
              case 2
                  X(i,:) =rand*(miu(1,:)-Best_X(1,:))+rand*Best_X(1,:);                  
              case 3
                  X(i,:) =rand*(miu(1,:)-pBest_X(i,:))+rand*pBest_X(i,:); 
           
           end 
           
        end
        % Check Position Bounds
         
         FU=X(i,:)>ub;FL=X(i,:)<lb;X(i,:)=(X(i,:).*(~(FU+FL)))+ub.*FU+lb.*FL;
  
        % Evaluation
          Cost(i) = fobj(X(i,:));
        % Update Personal Best
        if Cost(i)<pBest_Cost(i)     
            pBest_X(i,:) = X(i,:);
            pBest_Cost(i) = Cost(i);
            % Update Global Best
            if pBest_Cost(i)<Best_Cost
                Best_X = pBest_X(i,:);
                Best_Cost = pBest_Cost(i);
            end
        end
        
    end
    
    Convergence_curve(it)=Best_Cost;
    Average_curve(it)=sum(Cost)/max(size(Cost));
    % Show Information
    disp(['Iteration ' num2str(it) ': BestCost = ' num2str(Convergence_curve(it))]); 
end
