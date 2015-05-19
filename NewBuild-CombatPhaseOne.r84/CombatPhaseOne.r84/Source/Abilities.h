#ifndef MXOSIM_ABILITIES_H
#define MXOSIM_ABILITIES_H

#include "CharacterModel.h"



class Abilities
{

	public :
	
		typedef enum Ability_T
		{
				// Awakend Abilities.
			AWAKENED ,
			
			EVADE_COMBAT ,
			
			HYPER_JUMP_BETA ,
			
			HYPER_SENSE ,
			CALM_MIND_CALM_BODY ,
			
			HYPER_DEFLECT ,
			EMPTY_MIND ,
			
			HYPER_BLOCK ,
			CONSISTANT_TECHNIQUE ,
			
			HYPER_DODGE ,
			ADRENALINE_BOOSTER ,
			
			HYPER_SPRINT ,
			HYPER_JUMP ,
			HYPER_SPEED ,
			
			CHEAP_SHOT ,
			HEAD_BUTT ,
			
			CONCENTRATION ,
			DETERMINATION ,
			
			DETECTION ,
			LOCATE_DATA_NODE ,
			TAP_DATA_NODE ,
		} ;
		
		typedef enum AbilityUseTime_T
		{
			ANYTIME ,
			OUTSIDE_COMBAT ,
			CLOSE_COMBAT ,
		} ;
		
		typedef enum AbilityType_T
		{
			SPECIAL ,
			SELF_BUFF ,
			COMBAT ,
			DEBUFF ,
		} ;
		
		typedef enum TargetRequirement_T
		{
			NONE ,
			DAZED ,
		} ;
		
		typedef struct CommonAbilityAttributes_T
		{
			// Name string?
			Ability_T Requires ;
			unsigned RequireLevel ;
			
			bool Activatable ;
			
			unsigned MemoryCost ;
		} ;
		
		typedef struct AvtivatableAbilityAttributes_T
		{
			unsigned ISCost ;
			
				// In seconds, 0 represents no delay.
			unsigned ReuseDelay ;
		} ;
		
		typedef struct SelfBuffAttributes_T
		{
			// Attribute/skill/attribute being buffed
			// Amount being buffed.
		} ;
		
		typedef struct CombatAttributes_T
		{
			unsigned BaseDamage ;
			
			TargetRequirement_T TargetRequirement ;
			
			unsigned Range ;
			
			bool HasDebuff ;
		} ;
		
		/*typedef struct CombatAttributes_T
		{
			// Attribute/skill/attribute being buffed
			// Amount being buffed.
			// Combine with Self Buff?
		} ;*/
		
		static Abilities& Instance()
		{
			static Abilities singleton;
			return singleton;
		} ;
		
		// Map of abilities to ability attributes then separate maps of abilities to specific attributes, ex combat.  Use find to determine if ability is combat.

	private :

		Abilities ( void ) {} ;
		Abilities ( Abilities const& ) {} ;
		Abilities& operator= ( const Abilities ) {} ;
} ;

#endif MXOSIM_ABILITIES_H