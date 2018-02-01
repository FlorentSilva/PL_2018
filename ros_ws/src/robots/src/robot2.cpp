/**** Projet long N7 2017 ****/

#include "robot2.h"



//Constructeur
Robot2::Robot2()
{
	//Valeur de pi
	pi=3.14159265359;
	
	for (int i=0;i<7;i++)
	{
		//ATTENTION : Ne pas changer la valeur d'initialisation
		//l'ensemble des setModes doit être à 1 pour que les joints des robots soient commandés
		mymodes[i]=1;
	}
}



//Destructeur
Robot2::~Robot2()
{}



/*** Fonctions internes permettant le contrôle du robot ***/

/** Pour atteindre une position prédéfinie **/
//Fonction permettant d'envoyer le robot dans une position prédéfinie
void Robot2::EnvoyerRobot(int numposition)
{
	robotPosition.data=numposition;

	//Choix de la position en fonction du numéro reçu
	switch(numposition)
	{
		case 1:
			Rpos[0]=128*pi/180;
			Rpos[1]=90*pi/180;
			Rpos[2]=90*pi/180;
			Rpos[3]=80*pi/180;
			Rpos[4]=90*pi/180;
			Rpos[5]=-90*pi/180;
			Rpos[6]=-40*pi/180;
			break;

		case 2:
			Rpos[0]=60*pi/180;
			Rpos[1]=90*pi/180;
			Rpos[2]=90*pi/180;
			Rpos[3]=80*pi/180;
			Rpos[4]=90*pi/180;
			Rpos[5]=-90*pi/180;
			Rpos[6]=-20*pi/180;
			break;

		case 3:
			Rpos[0]=15*pi/180;
			Rpos[1]=90*pi/180;
			Rpos[2]=90*pi/180;
			Rpos[3]=80*pi/180;
			Rpos[4]=90*pi/180;
			Rpos[5]=-90*pi/180;
			Rpos[6]=-65*pi/180;
			break;
	
		case 4:
			Rpos[0]=-53*pi/180;
			Rpos[1]=90*pi/180;
			Rpos[2]=90*pi/180;
			Rpos[3]=80*pi/180;
			Rpos[4]=90*pi/180;
			Rpos[5]=-90*pi/180;
			Rpos[6]=-40*pi/180;
			break;
	}
	
	
	//Utilisation du service simRosSetJointState pour envoyer le robot dans la position souhaitée
	std::vector<int> myhandles(Rints, Rints+sizeof(Rints)/sizeof(int));	
	std::vector<unsigned char> mysetmodes(mymodes, mymodes+sizeof(mymodes)/sizeof(unsigned char));
	std::vector<float> myvalues(Rpos, Rpos+sizeof(Rpos)/sizeof(float));

	srv_simRosSetJoint.request.handles = myhandles;
	srv_simRosSetJoint.request.setModes = mysetmodes;
	srv_simRosSetJoint.request.values = myvalues;
	
	client_simRosSetJoint.call(srv_simRosSetJoint);

	//Vérification après l'appel du service
	if(srv_simRosSetJoint.response.result==-1)
	{
		ROS_INFO("Position predefinie %d pour le robot 2 non atteinte", numposition);

		//Retour vers la commande 
		retour.data = 1;
		pub_retourCommande.publish(retour);
	}
	else
	{	
		//Utilisation du service simRosGetJointState pour savoir si le mouvement du robot est terminé : 
		//on considère que la position est atteinte si la position est à 0.001 de sa consigne
		float Position;
		for(int i=0;i<7;i++)
		{
			Position=0;

			//Appel du service pour connaître la position du robot
			srv_simRosGetJoint.request.handle = Rints[i];
			client_simRosGetJoint.call(srv_simRosGetJoint);
			Position = srv_simRosGetJoint.response.state.position[0];

			//Attente jusqu'à ce que la position soit atteinte
			while(std::abs(Position-Rpos[i])>=0.001)
			{
				srv_simRosGetJoint.request.handle = Rints[i];
				client_simRosGetJoint.call(srv_simRosGetJoint);
				Position = srv_simRosGetJoint.response.state.position[0];	
			}
		}

		ROS_INFO("Position atteinte");

		//Retour vers la commande
		retour.data = 2;
		pub_retourCommande.publish(retour);
		
		//Retour de la position actuelle du robot
		pub_robotPosition.publish(robotPosition);
	}

}



/** Pour atteindre une position définie manuellement **/
//Fonction permettant d'envoyer le robot dans une position définie manuellement
void Robot2::EnvoyerJoints(int joint1, int joint2, int joint3, int joint4, int joint5, int joint6, int joint7)
{
	//Récupération des angles choisis par l'utilisateur
	Rpos[0]=(joint1)*pi/180;
	Rpos[1]=(joint2)*pi/180;
	Rpos[2]=(joint3)*pi/180;
	Rpos[3]=(joint4)*pi/180;
	Rpos[4]=(joint5)*pi/180;
	Rpos[5]=(joint6)*pi/180;
	Rpos[6]=(joint7)*pi/180;

		
	//Utilisation du service simRosSetJointState our envoyer le robot dans la position souhaitée
	std::vector<int> myhandles(Rints, Rints+sizeof(Rints)/sizeof(int));	
	std::vector<unsigned char> mysetmodes(mymodes, mymodes+sizeof(mymodes)/sizeof(unsigned char));
	std::vector<float> myvalues(Rpos, Rpos+sizeof(Rpos)/sizeof(float));

	srv_simRosSetJoint.request.handles = myhandles;
	srv_simRosSetJoint.request.setModes = mysetmodes;
	srv_simRosSetJoint.request.values = myvalues;
	
	client_simRosSetJoint.call(srv_simRosSetJoint);

	//Vérification après l'appel du service
	if(srv_simRosSetJoint.response.result==-1)
	{
		ROS_INFO("Position definie manuellement pour le robot 2 non atteinte");

		//Retour vers la commande
		retour.data = 1;
		pub_retourCommande.publish(retour);
	}
	else
	{	
		//Utilisation du service simRosGetJointState pour savoir si le mouvement du robot est terminé
		//On considère que la position est atteinte si la position est à 0.001 de sa consigne
		float Position;
		for(int i=0;i<7;i++)
		{
			Position=0;

			//Appel du service pour connaître la position du robot
			srv_simRosGetJoint.request.handle = Rints[i];
			client_simRosGetJoint.call(srv_simRosGetJoint);
			Position = srv_simRosGetJoint.response.state.position[0];

			//Attente jusqu'à ce que la position soit atteinte
			while(std::abs(Position-Rpos[i])>=0.001)
			{
				srv_simRosGetJoint.request.handle = Rints[i];
				client_simRosGetJoint.call(srv_simRosGetJoint);
				Position = srv_simRosGetJoint.response.state.position[0];	
			}
		}

		ROS_INFO("Position atteinte");

		//Retour vers la commande
		retour.data = 2;
		pub_retourCommande.publish(retour);
	}
}



/** Pour descendre ou monter le bras **/
//Fonction permettant de mettre le bras en position basse
void Robot2::DescendreBras()
{
	//Récupération et modification de la position actuelle
	Rpos[0]=Rpos[0]+2*pi/180;
	Rpos[1]=Rpos[1]+10*pi/180;
	Rpos[2]=Rpos[2];
	Rpos[3]=Rpos[3];
	Rpos[4]=Rpos[4]+10*pi/180;
	Rpos[5]=Rpos[5]+3*pi/180;
	Rpos[6]=Rpos[6]+6*pi/180;

	//Utilisation du service simRosSetJointState pour envoyer le robot dans la position souhaitée
	std::vector<int> myhandles(Rints, Rints+sizeof(Rints)/sizeof(int));	
	std::vector<unsigned char> mysetmodes(mymodes, mymodes+sizeof(mymodes)/sizeof(unsigned char));
	std::vector<float> myvalues(Rpos, Rpos+sizeof(Rpos)/sizeof(float));

	srv_simRosSetJoint.request.handles = myhandles;
	srv_simRosSetJoint.request.setModes = mysetmodes;
	srv_simRosSetJoint.request.values = myvalues;
	
	client_simRosSetJoint.call(srv_simRosSetJoint);

	//Vérification après l'appel du service
	if(srv_simRosSetJoint.response.result==-1)
	{
		ROS_INFO("Bras non descendu pour le robot 2");

		//Retour vers la commande
		retour.data = 3;
		pub_retourCommande.publish(retour);
	}
	else
	{	
		//Utilisation du service simRosGetJointState pour savoir si le mouvement du robot est terminé
		//On considère que la position est atteinte si la position est à 0.001 de sa consigne
		float Position;
		for(int i=0;i<7;i++)
		{
			Position=0;
			
			//Appel du service pour connaître la position du robot
			srv_simRosGetJoint.request.handle = Rints[i];
			client_simRosGetJoint.call(srv_simRosGetJoint);
			Position = srv_simRosGetJoint.response.state.position[0];

			//Attente jusqu'à ce que la position soit atteinte
			while(std::abs(Position-Rpos[i])>=0.001)
			{
				srv_simRosGetJoint.request.handle = Rints[i];
				client_simRosGetJoint.call(srv_simRosGetJoint);
				Position = srv_simRosGetJoint.response.state.position[0];	
			}
		}

		ROS_INFO("Bras descendu pour le robot 2");

		//Retour vers la commande
		retour.data = 4;
		pub_retourCommande.publish(retour);

		//Retour de l'état actuel du bras
		robotBras.data = 0;
		pub_robotBras.publish(robotBras);
	}
}



//Fonction permettant de mettre le bras en position haute 
void Robot2::MonterBras()
{
	//Récupération et modification de la position actuelle
	Rpos[0]=Rpos[0]-2*pi/180;
	Rpos[1]=Rpos[1]-10*pi/180;
	Rpos[2]=Rpos[2];
	Rpos[3]=Rpos[3];
	Rpos[4]=Rpos[4]-10*pi/180;
	Rpos[5]=Rpos[5]-3*pi/180;
	Rpos[6]=Rpos[6]-6*pi/180;

	//Utilisation du service simRosSetJointState pour envoyer le robot dans la position souhaitée
	std::vector<int> myhandles(Rints, Rints+sizeof(Rints)/sizeof(int));	
	std::vector<unsigned char> mysetmodes(mymodes, mymodes+sizeof(mymodes)/sizeof(unsigned char));
	std::vector<float> myvalues(Rpos, Rpos+sizeof(Rpos)/sizeof(float));

	srv_simRosSetJoint.request.handles = myhandles;
	srv_simRosSetJoint.request.setModes = mysetmodes;
	srv_simRosSetJoint.request.values = myvalues;
	
	client_simRosSetJoint.call(srv_simRosSetJoint);


	//Vérification après l'appel du service
	if(srv_simRosSetJoint.response.result==-1)
	{
		ROS_INFO("Bras non monte pour le robot 2");

		//Retour vers la commande
		retour.data = 3;
		pub_retourCommande.publish(retour);	
	}
	else
	{	
		//Utilisation du service simRosGetJointState pour savoir si le mouvement du robot est terminé 
		//On considère que la position est atteinte si la position est à 0.001 de sa consigne
		float Position;
		for(int i=0;i<7;i++)
		{
			Position=0;

			//Appel du service pour connaître la position du robot
			srv_simRosGetJoint.request.handle = Rints[i];
			client_simRosGetJoint.call(srv_simRosGetJoint);
			Position = srv_simRosGetJoint.response.state.position[0];

			//Attente jusqu'à ce que la position soit atteinte
			while(std::abs(Position-Rpos[i])>=0.001)
			{
				srv_simRosGetJoint.request.handle = Rints[i];
				client_simRosGetJoint.call(srv_simRosGetJoint);
				Position = srv_simRosGetJoint.response.state.position[0];	
			}
		}

		ROS_INFO("Bras monte pour le robot 2");

		//Retour vers la commande
		retour.data = 5;
		pub_retourCommande.publish(retour);

		//Retour de l'état actuel du bras
		robotBras.data = 1;
		pub_robotBras.publish(robotBras);
	}
}



/** Pour fermer ou ouvrir la pince **/
//Fonction permettant de fermer la pince du robot en envoyant une commande sur le topic correspondant
void Robot2::FermerPince()
{
	//Message de commande de fermeture de la pince (1 pour fermer, 0 pour ouvrir)
	std_msgs::Int32 cmd;
	cmd.data = 1;

	pub_pince.publish(cmd);

	//Si la pince n'est pas déjà fermée, un retour est envoyé à la commande
	if(retour.data != 6)
	{
		//Attente pour que la pince se ferme
		float t0, time;
		client_simGetVrepTime.call(srv_simGetVrepTime);
		t0 = srv_simGetVrepTime.response.simulationTime;
		time = t0;
		while(time - t0 < 2)
		{
			client_simGetVrepTime.call(srv_simGetVrepTime);
			time = srv_simGetVrepTime.response.simulationTime;
		}
	
		//Retour vers la commande
		retour.data = 6;
		pub_retourCommande.publish(retour);
	}

	//Retour de l'état de la pince
	robotPince.data=1;
	pub_robotPince.publish(robotPince);
}



//Fonction permettant d'ouvrir la pince du robot en envoyant une commande sur le topic correspondant
void Robot2::OuvrirPince()
{
	//Message de commande d'ouverture de la pince (1 pour fermer, 0 pour ouvrir)
	std_msgs::Int32 cmd;
	cmd.data = 0;

	pub_pince.publish(cmd);


	//Si la pince n'est pas déjà ouverte, un retour est envoyé à la commande
	if(retour.data != 7)
	{
		//Attente pour que la pince s'ouvre
		float t0, time;
		client_simGetVrepTime.call(srv_simGetVrepTime);
		t0 = srv_simGetVrepTime.response.simulationTime;
		time = t0;
		while(time - t0 < 1)
		{
			client_simGetVrepTime.call(srv_simGetVrepTime);
			time = srv_simGetVrepTime.response.simulationTime;
		}
	
		//Retour vers la commande
		retour.data = 7;
		pub_retourCommande.publish(retour);
	}

	//Retour de l'état de la pince
	robotPince.data=0;
	pub_robotPince.publish(robotPince);
}





/*** Fonctions permettant de controler le robot avec des ordres du noeud commande ***/

/** Envoyer le robot automatiquement **/
//Fonction Callback permettant d'envoyer le robot dans une position prédéfinie à la réception du message de Commande 
void Robot2::SendPositionCallback(const std_msgs::Int32::ConstPtr& msg)
{
	//Récupération des données du message : numéro de la position prédéfinie
	int pos;
	pos = msg->data;

	//Envoi du robot dans la position choisie
	EnvoyerRobot(pos);
}



/** Envoyer le robot manuellement **/
//Fonction Callback permettant d'envoyer le robot dans une position choisie par l'utilisateur à la réception du message de Commande 
void Robot2::SendJointsCallback(const robots::RobotJoints::ConstPtr& msg)
{
	EnvoyerJoints(msg->joint1, msg->joint2, msg->joint3, msg->joint4, msg->joint5, msg->joint6, msg->joint7);
}



/** Fermer la pince **/
//Fonction Callback permettant de fermer la pince du robot à la réception du message de Commande 
void Robot2::FermerPinceCallback(const std_msgs::Int32::ConstPtr& msg)
{
	FermerPince();
}



/** Ouvrir la pince **/
//Fonction Callback permettant d'ouvrir la pince du robot à la réception du message de Commande 
void Robot2::OuvrirPinceCallback(const std_msgs::Int32::ConstPtr& msg)
{
	OuvrirPince();	
}



/** Descendre le bras **/
//Fonction Callback permettant de mettre le bras en position basse à la réception du message de Commande 
void Robot2::DescendreBrasCallback(const std_msgs::Int32::ConstPtr& msg)
{
	DescendreBras();
}



/** Monter le bras **/
//Fonction Callback permettant de mettre le bras en position haute à la réception du message de Commande 
void Robot2::MonterBrasCallback(const std_msgs::Int32::ConstPtr& msg)
{
	MonterBras();
}


/** Contrôler le robot entièrement **/
//Fonction Callback permettant de contrôler l'ensemble des mouvements du robot à la réception du message de Commande
void Robot2::ControlerRobotCallback(const robots::MoveRobot::ConstPtr& msg)
{
	//Envoi du robot dans la position souhaitée
	EnvoyerRobot(msg->position);

	//Envoi du bras dans l'état souhaité
	switch(msg->bras)
	{
		case -1:
			DescendreBras();
			break;

		case 1:
			MonterBras();
			break;
	}
	
	//Envoi de la pince dans l'état souhaité
	switch(msg->pince)
	{
		case -1:
			OuvrirPince();
			break;

		case 1:
			FermerPince();
			break;
	}
}




/*** Initialisation ***/

//Initialisation des services, des publishers et des subscribers + Récupération des handles des robots
void Robot2::init(ros::NodeHandle noeud)
{
	//Déclaration service simRosGetObjectHandle
	client_simRosGetHandle = noeud.serviceClient<vrep_common::simRosGetObjectHandle>("/vrep/simRosGetObjectHandle");

	//Déclaration service simRosSetJointState
	client_simRosSetJoint = noeud.serviceClient<vrep_common::simRosSetJointState>("/vrep/simRosSetJointState");

	//Déclaration service simRosGetJointState
	client_simRosGetJoint = noeud.serviceClient<vrep_common::simRosGetJointState>("/vrep/simRosGetJointState");

	//Déclaration service simRosGetInfo
	client_simGetVrepTime = noeud.serviceClient<vrep_common::simRosGetInfo>("/vrep/simRosGetInfo");


	//Subscribers
	planifSendPosition = noeud.subscribe("/commande/Simulation/SendPositionRobot2",10,&Robot2::SendPositionCallback,this);

	planifSendJoints = noeud.subscribe("/commande/Simulation/SendJointsRobot2",10,&Robot2::SendJointsCallback,this);
	
 	planifFermerPince = noeud.subscribe("/commande/Simulation/FermerPinceRobot2",10,&Robot2::FermerPinceCallback,this);

	planifOuvrirPince = noeud.subscribe("/commande/Simulation/OuvrirPinceRobot2",10,&Robot2::OuvrirPinceCallback,this);
	
	planifDescendreBras = noeud.subscribe("/commande/Simulation/DescendreBras2",10,&Robot2::DescendreBrasCallback,this);

	planifMonterBras = noeud.subscribe("/commande/Simulation/MonterBras2",10,&Robot2::MonterBrasCallback,this);

	planifControlerRobot = noeud.subscribe("/commande/Simulation/ControlerBras2",10,&Robot2::ControlerRobotCallback,this);


	//Publishers
	pub_pince = noeud.advertise<std_msgs::Int32>("/robot/cmdPinceRobot2", 1);

	pub_robotPosition = noeud.advertise<std_msgs::Int32>("/robot/PositionRobot2",1);
	pub_robotBras = noeud.advertise<std_msgs::Int32>("/robot/BrasRobot2",1);
	pub_robotPince = noeud.advertise<std_msgs::Int32>("/robot/PinceRobot2",1);

	pub_retourCommande = noeud.advertise<std_msgs::Int32>("/commande/Simulation/retourCommande2", 1);
	

	//Utilisation du service simRosGetObjectHandle pour obtenir les handles du robot 2
	for (int i=1;i<8;i++)
	{
		std::stringstream sr;	
		sr << i;
		srv_simRosGetHandle.request.objectName="LBR_iiwa_14_R820_joint" + sr.str() + "#0";

		client_simRosGetHandle.call(srv_simRosGetHandle);
		
		Rints[i-1]=srv_simRosGetHandle.response.handle;
		
		if(Rints[i-1]==-1)
		{
			ROS_INFO("Robot 2 : Handle non obtenu pour joint %d",i);
		}
		else
		{
			if(i==7)
			{
				ROS_INFO("Robot 2 OK");
				//Retour vers la commande
				retour.data = 0;
				pub_retourCommande.publish(retour);
			}
		} 
	}	

	
}




