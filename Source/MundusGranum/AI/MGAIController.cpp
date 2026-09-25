// Fill out your copyright notice in the Description page of Project Settings.


#include "MGAIController.h"

#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"


AMGAIController::AMGAIController()
{
	Blackboard = CreateDefaultSubobject<UBlackboardComponent>("BlackboardComp");
	check(Blackboard);
	BTComp = CreateDefaultSubobject<UBehaviorTreeComponent>("BTComp");
	check(BTComp);
}
