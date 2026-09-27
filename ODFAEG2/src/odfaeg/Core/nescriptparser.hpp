namespace odfaeg {
    namespace core {
        class Parser {
            void parseFile(std::string fileName);
            void parseNode(ASTNode& parent, ASTNode& node, std::string content);
            void parseInstructions(uint32_t& currentPos, ASTNode node);
        }; 
    }
}