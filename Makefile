SRC_MAIN 	= 	main.cpp \
				src/ClientSocket.cpp \
				src/ListenSocket.cpp \
				src/Server.cpp \
				src/Channel.cpp \


SRC			= ${SRC_MAIN}

OBJ			= ${SRC:.cpp=.o}

HEADER		= inc

CC 			= c++

RM 			= rm -f

CPPFLAGS 	= -Wall -Werror -Wextra -std=c++98 -Wno-unused-parameter

NAME 		= ircserv

ifdef DEBUG
	CPPFLAGS += -fsanitize=address -g3
endif

all:		$(NAME)

.cpp.o:
			@$(CC) $(CPPFLAGS) -I $(HEADER) -c $< -o $(<:.cpp=.o)

$(NAME):	$(OBJ)
			@$(CC) $(CPPFLAGS) $(OBJ) -I $(HEADER) -o $(NAME)

debug:
			${MAKE} DEBUG=1

clean:
			@$(RM) $(OBJ)

fclean: 	clean
			@$(RM) $(NAME)

re:			fclean all

.PHONY: 	all clean fclean re
