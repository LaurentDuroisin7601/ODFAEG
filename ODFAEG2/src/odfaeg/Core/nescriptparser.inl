namespace odfaeg {
    namespace core {        
        void Pasrer::parseFile() { 
            ASTNode root;
            root.blocStart = 0;
            root.blocEnd = content.size();
            if (content.find("{") != std::string::npos) {
                ASTNode node;
                node.blocStart = content.find("{") + 1;                
                parseNode(root, node, content);    
            }
        }
        void Parser::parseNode(ASTNode& parent, ASTNode& node, std::string content) { 
            uint32_t openBracketPos = content.find(node.bloclStart, "{"); 
            uint32_t closedBracketPos = content.find(currentPos, "}");            
            //Gestion des sous blocs.
            while (openBracketPos != std::string::npos 
                    && closedBracketPos != std::string::npos 
                    && openBracketPos < closesdBracketPos) {                    
                ASTNode subNode;
                subNode.startPos = openBracketPos + 1;
                parseNode(node, subNode, content);
                openBracketPos = content.find(node.bloclStart, "{"); 
                closedBracketPos = content.find(closedBracketPos+1, "}");
                node.blocEnd = closedBrackePos - 1;
                parent.addChild(node);                
            }
            node.content = content.substr(node.blocStart, node.blocEnd);
            parent.addChild(node);
            //Noeuds frères.
            if (content.find(node.startPos, "{") != std::string::npos) {
                parseNode(parent, node, content);
            }
        } 
    }
}