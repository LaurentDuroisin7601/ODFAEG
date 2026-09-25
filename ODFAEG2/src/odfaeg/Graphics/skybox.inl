namespace odfaeg {
    namespace graphic {
        Skybox::Skybox(float size, std::vector<std::string> filepath) {
            skyboxCM.loadCubeMapFromFile(filepaths);            //Droite.

            VertexBuffer vb(GPUContext::instance().getDevice(), Triangles);
            vb.resize(24, 36);
            //Droite.
            Vertex v1(math::Vec3f(size*0.5f, -size*0.5f, -size*0.5f));
            Vertex v2(math::Vec3f(size*0.5f, size*0.5f, -size*0.5f));
            Vertex v3(math::Vec3f(size*0.5f, size*0.5f, size*0.5f));
            Vertex v4(math::Vec3f(size*0.5f, -size*0.5f, size*0.5f));
            vb[0] = v1;
            vb[1] = v2;
            vb[2] = v3;
            vb[3] = v4;
            vb.setIndex(0, 0);
            vb.setIndex(1, 1);
            vb.setIndex(2, 2);
            vb.setIndex(3, 0);
            vb.setIndex(4, 2);
            vb.setIndex(5, 3);
            //Gauche.           
            Vertex v5(math::Vec3f(-size*0.5f, -size*0.5f, -size*0.5f));
            Vertex v6(math::Vec3f(-size*0.5f, size*0.5f, -size*0.5f));
            Vertex v7(math::Vec3f(-size*0.5f, size*0.5f, size*0.5f));
            Vertex v8(math::Vec3f(-size*0.5f, -size*0.5f, size*0.5f));
            vb[4] = v5;
            vb[5] = v6;
            vb[6] = v7;
            vb[7] = v8;
            vb.setIndex(6, 4);
            vb.setIndex(7, 5);
            vb.setIndex(8, 6);
            vb.setIndex(9, 4);
            vb.setIndex(10, 6);
            vb.setIndex(11, 7);            
            //Dessus           
            Vertex v9(math::Vec3f(-size*0.5f, size*0.5f, -size*0.5f));
            Vertex v10(math::Vec3f(size*0.5f, size*0.5f, -size*0.5f));
            Vertex v11(math::Vec3f(size*0.5f, size*0.5f, size*0.5f));
            Vertex v12(math::Vec3f(-size*0.5f, size*0.5f, size*0.5f));
            vb[8] = v9;
            vb[9] = v10;
            vb[10] = v11;
            vb[11] = v12;
            vb.setIndex(12, 8);
            vb.setIndex(13, 9);
            vb.setIndex(14, 10);
            vb.setIndex(15, 8);
            vb.setIndex(16, 10);
            vb.setIndex(17, 11);  
            
            //Dessous.            
            Vertex v13(math::Vec3f(-size*0.5f, -size*0.5f, -size*0.5f));
            Vertex v14(math::Vec3f(size*0.5f, -size*0.5f, -size*0.5f));
            Vertex v15(math::Vec3f(size*0.5f, -size*0.5f, size*0.5f));
            Vertex v16(math::Vec3f(-size*0.5f, -size*0.5f, size*0.5f));
            vb[12] = v13;
            vb[13] = v14;
            vb[14] = v15;
            vb[15] = v16;
            vb.setIndex(18, 12);
            vb.setIndex(19, 13);
            vb.setIndex(20, 14);
            vb.setIndex(21, 12);
            vb.setIndex(22, 14);
            vb.setIndex(23, 15);
            /*for (unsigned int i = 0; i < face4.getVertexArray().getVertexCount(); i++) {
                //////std::cout<<"vertex position : "<<face4.getVertexArray()[i].position.x<<std::endl;
            }*/
            //Devant            
            Vertex v17(math::Vec3f(-size*0.5f, -size*0.5f, size*0.5f));
            Vertex v18(math::Vec3f(size*0.5f, -size*0.5f, size*0.5f));
            Vertex v19(math::Vec3f(size*0.5f, size*0.5f, size*0.5f));
            Vertex v20(math::Vec3f(-size*0.5f, size*0.5f, size*0.5f));
            vb[16] = v17;
            vb[17] = v18;
            vb[18] = v19;
            vb[19] = v20;           
            vb.setIndex(24, 16);
            vb.setIndex(25, 17);
            vb.setIndex(26, 18);
            vb.setIndex(27, 17);
            vb.setIndex(28, 18);
            vb.setIndex(29, 19);

            //Derrière.
            Vertex v21(math::Vec3f(-size*0.5f, -size*0.5f, -size*0.5f));
            Vertex v22(math::Vec3f(size*0.5f, -size*0.5f, -size*0.5f));
            Vertex v23(math::Vec3f(size*0.5f, size*0.5f, -size*0.5f));
            Vertex v24(math::Vec3f(-size*0.5f, size*0.5f, -size*0.5f));
            vb[20] = v21;
            vb[21] = v22;
            vb[22] = v23;
            vb[23] = v24;
            vb.setIndex(30, 20);
            vb.setIndex(31, 21);
            vb.setIndex(32, 22);
            vb.setIndex(33, 20);
            vb.setIndex(34, 22);
            vb.setIndex(35, 23);           
        }
        VertexBuffer& Skybox::getVertexBuffer() {
            return skyboxVB;
        }
        Texture& Skybox::getTexture() {
            return skyboxCM;
        } 
    }   
}