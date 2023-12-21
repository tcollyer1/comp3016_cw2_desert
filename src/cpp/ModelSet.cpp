//#include "..\h\ModelSet.h" // TODO: Causes conflicts as this is included in main.cpp as well, because of the LOGL model file...

//// Model max scaling values
//#define TREE_MAX	0.005f
//#define CACTUS_MAX	0.005f
//#define GRASS_MAX	0.01f
//
//void ModelSet::drawModels(MVP* mvp)
//{
//	// Draw grass biome models (grass, cacti)
//	for (int i = 0; i < grassModPos.size(); i++)
//	{
//		mvp->resetModel();
//
//		modelPos.x = TERRAIN_START.x + grassModPos[i].x;
//		modelPos.y = TERRAIN_START.y + grassModPos[i].y;
//		modelPos.z = TERRAIN_START.z + grassModPos[i].z;
//
//		// Set starting model position
//		mvp->moveModel(modelPos);
//
//		// Draw model using enabled shaders
//		if (terrain->getModelType(i))
//		{
//			// Scale down & rotate cactus object, draw cactus;
//			mvp->scaleModel(vec3((float)(CACTUS_MAX / terrain->getScale(i))));
//			mvp->rotateModel((float)terrain->getRotation(i), vec3(0.0f, 1.0f, 0.0f));
//
//			// Set our MVP matrix to the uniform variables
//			setMVP(mvp);
//
//			cactus->Draw((*shaders));
//		}
//		else
//		{
//			// Scale down & rotate cactus object, draw cactus;
//			mvp->scaleModel(vec3((float)(GRASS_MAX / terrain->getScale(i))));
//			mvp->rotateModel(90.0f, vec3(0.0f, 1.0f, 0.0f));
//
//			// Set our MVP matrix to the uniform variables
//			setMVP(mvp);
//
//			grass->Draw((*shaders));
//		}
//	}
//
//	// Draw desert oasis biome models (trees/grass)
//	for (int i = 0; i < oasisModPos.size(); i++)
//	{
//		mvp->resetModel();
//
//		modelPos.x = TERRAIN_START.x + oasisModPos[i].x;
//		modelPos.y = TERRAIN_START.y + oasisModPos[i].y;
//		modelPos.z = TERRAIN_START.z + oasisModPos[i].z;
//
//		// Set starting model position
//		mvp->moveModel(modelPos);
//
//		if (terrain->getModelType(i))
//		{
//			// Scale down & rotate tree object. Only one set size used for tree
//			mvp->scaleModel(vec3(TREE_MAX));
//			mvp->rotateModel((float)terrain->getRotation(i), vec3(0.0f, 1.0f, 0.0f));
//
//			// Set our MVP matrix to the uniform variables
//			setMVP(mvp);
//
//			tree->Draw((*shaders));
//		}
//		else
//		{
//			// Scale down & rotate cactus object, draw cactus;
//			mvp->scaleModel(vec3((float)(GRASS_MAX / terrain->getScale(i))));
//			mvp->rotateModel(90.0f, vec3(0.0f, 1.0f, 0.0f));
//
//			// Set our MVP matrix to the uniform variables
//			setMVP(mvp);
//
//			grass->Draw((*shaders));
//		}
//	}
//}