NAME    := webserv
INCDIR  := include
SRCDIR  := src
OBJDIR  := obj

SRC     := $(SRCDIR)/config/ConfigParser.cpp     \
		   $(SRCDIR)/config/Location.cpp         \
		   $(SRCDIR)/config/ServerConfig.cpp     \
		   $(SRCDIR)/http/HttpRequest.cpp        \
		   $(SRCDIR)/http/HttpResponse.cpp        \
		   $(SRCDIR)/http/HttpUtils.cpp          \
		   $(SRCDIR)/http/RequestParser.cpp      \
		   $(SRCDIR)/http/ResponseBuilder.cpp    \
		   $(SRCDIR)/http/handlers/AHandler.cpp  \
		   $(SRCDIR)/http/handlers/GetHandler.cpp \
		   $(SRCDIR)/http/handlers/PostHandler.cpp \
		   $(SRCDIR)/http/handlers/DeleteHandler.cpp \
		   $(SRCDIR)/server/Server.cpp           \
		   $(SRCDIR)/server/PollManager.cpp      \
		   $(SRCDIR)/server/Connexion.cpp        \
		   $(SRCDIR)/main.cpp					 \
		   
		
OBJ     := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRC))

CXX     := c++
#CXXFLAGS := -Wall -Wextra -Werror -std=c++98 -I$(INCDIR)
CXXFLAGS := -I$(INCDIR)

GREEN   := \033[1;32m
CYAN    := \033[1;36m
RESET   := \033[0m
SMILEY  := 👍
BROOM   := 🧹
SOAP    := 🧼

all: $(NAME)

# $(OBJDIR)/%.o: $(SRCDIR)/%.cpp
# 	@mkdir -p $(OBJDIR)
# 	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(NAME): $(OBJ)
	@$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)
	@echo "$(GREEN)✔ Build successful ! $(SMILEY)$(RESET)"

clean:
	@rm -rf $(OBJDIR)
	@echo "$(CYAN)$(BROOM) Object files cleaned.$(RESET)"

fclean: clean
	@rm -f $(NAME)
	@echo "$(CYAN)$(SOAP) Full cleanup done.$(RESET)"

re: fclean all

.PHONY: all clean fclean re