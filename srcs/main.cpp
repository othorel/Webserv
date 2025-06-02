/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmatkows <lmatkows@student.42perpignan.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/02 10:44:47 by lmatkows          #+#    #+#             */
/*   Updated: 2025/06/02 12:02:20 by lmatkows         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#include "./include/ConfigParser.hpp"

int	main(int argc, char **argv)
{
	if (argc!= 2)
		return;
// verif si argv 1 est bien un fichier
//parsing fichier config
	ConfigParser Config(argv[1]);

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	// gestion d'erreurs socket
    struct sockaddr_in serv_addr;

	// a integrer dans une classe setup serv
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(Config.getPort());// OLIVIER
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	// gestion d'erreurs bind
    listen(sockfd, SOMAXCONN); 
	//
	
	// A Voir si on se laisse la possibilite de gerer la longueur de la file d'attente
	// gestion d'erreurs listen

	// pour chaque client : 
		struct sockaddr_in client_addr;
	
	while
}