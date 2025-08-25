#include <algorithm>
#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "ontologenius/API/ontologenius/OntologyManipulator.h"

/*
│   ├──BinarySensor
│   │   ├── + Sensor
hasData allValuesFrom (oneOf ())=>
-0 data_prop(hasData) rest(0) 
 -1 
 c : 0 o : 0 d : 1 i : 0 cwa : 0
│   │   ├── = hasData allValuesFrom (oneOf ())
*/

onto::OntologyManipulator* onto_ptr;

TEST(reasoning_anonymous_class, no_datatype)
{
  std::vector<std::string> res;

  res = onto_ptr->individuals.getUp("garfield", 1);
  EXPECT_EQ(res.size(), 1);
  EXPECT_TRUE(std::find(res.begin(), res.end(), "Cat") != res.end());

  // test if no inference exist at initialization
  res = onto_ptr->individuals.getUp("alice", 1);
  EXPECT_EQ(res.size(), 1);
  EXPECT_TRUE(std::find(res.begin(), res.end(), "Human") != res.end());

  onto_ptr->feeder.addRelation("alice", "ownPet", "garfield");
  onto_ptr->feeder.waitUpdate(1000);

  res = onto_ptr->individuals.getUp("alice", 1);
  EXPECT_EQ(res.size(), 5);
  EXPECT_TRUE(std::find(res.begin(), res.end(), "GarfieldOwner") != res.end());     // ownPet value garfield
  EXPECT_TRUE(std::find(res.begin(), res.end(), "PetLover") != res.end());          // ((ownPet min 1) and (ownPet max 3))
  EXPECT_TRUE(std::find(res.begin(), res.end(), "ExclusiveCatOwner") != res.end()); // ownPet only Cat
  EXPECT_TRUE(std::find(res.begin(), res.end(), "PetOwner") != res.end());          // ownPet some (Cat or {rex, pongo})

  onto_ptr->feeder.addRelation("alice", "ownPet", "felix");
  onto_ptr->feeder.waitUpdate(1000);

  res = onto_ptr->individuals.getUp("alice", 1);
  EXPECT_EQ(res.size(), 6);
  EXPECT_TRUE(std::find(res.begin(), res.end(), "GarfieldOwner") != res.end());     // ownPet value garfield
  EXPECT_TRUE(std::find(res.begin(), res.end(), "PetLover") != res.end());          // ((ownPet min 1) and (ownPet max 3))
  EXPECT_TRUE(std::find(res.begin(), res.end(), "ExclusiveCatOwner") != res.end()); // ownPet only Cat
  EXPECT_TRUE(std::find(res.begin(), res.end(), "PetOwner") != res.end());          // ownPet some (Cat or {rex, pongo})
  EXPECT_TRUE(std::find(res.begin(), res.end(), "PerfectCatOwner") != res.end());   // (PetOwner and (ownPet exactly 2 {felix, garfield, duchesse}))

  onto_ptr->feeder.addRelation("alice", "ownPet", "duchesse");
  onto_ptr->feeder.waitUpdate(1000);

  res = onto_ptr->individuals.getUp("alice", 1);
  EXPECT_EQ(res.size(), 6);
  EXPECT_TRUE(std::find(res.begin(), res.end(), "GarfieldOwner") != res.end());     // ownPet value garfield
  EXPECT_TRUE(std::find(res.begin(), res.end(), "PetLover") != res.end());          // ((ownPet min 1) and (ownPet max 3))
  EXPECT_TRUE(std::find(res.begin(), res.end(), "ExclusiveCatOwner") != res.end()); // ownPet only Cat
  EXPECT_TRUE(std::find(res.begin(), res.end(), "PetOwner") != res.end());          // ownPet some (Cat or {rex, pongo})
  EXPECT_TRUE(std::find(res.begin(), res.end(), "CrazyCatOwner") != res.end());     // (PetOwner and (ownPet min 3 Cat))

  onto_ptr->feeder.addRelation("alice", "ownPet", "rex");
  onto_ptr->feeder.waitUpdate(1000);

  res = onto_ptr->individuals.getUp("alice", 1);
  EXPECT_EQ(res.size(), 4);
  EXPECT_TRUE(std::find(res.begin(), res.end(), "GarfieldOwner") != res.end());     // ownPet value garfield
  EXPECT_TRUE(std::find(res.begin(), res.end(), "PetOwner") != res.end());          // ownPet some (Cat or {rex, pongo})
  EXPECT_TRUE(std::find(res.begin(), res.end(), "CrazyCatOwner") != res.end());     // (PetOwner and (ownPet min 3 Cat))

  onto_ptr->feeder.removeRelation("alice", "ownPet", "duchesse");
  onto_ptr->feeder.waitUpdate(1000);

  res = onto_ptr->individuals.getUp("alice", 1);
  EXPECT_EQ(res.size(), 5);
  EXPECT_TRUE(std::find(res.begin(), res.end(), "GarfieldOwner") != res.end());     // ownPet value garfield
  EXPECT_TRUE(std::find(res.begin(), res.end(), "PetOwner") != res.end());          // ownPet some (Cat or {rex, pongo})
  EXPECT_TRUE(std::find(res.begin(), res.end(), "PetLover") != res.end());          // ((ownPet min 1) and (ownPet max 3))
  EXPECT_TRUE(std::find(res.begin(), res.end(), "PerfectCatOwner") != res.end());   // (PetOwner and (ownPet exactly 2 {felix, garfield, duchesse}))

  onto_ptr->feeder.removeRelation("alice", "ownPet", "felix");
  onto_ptr->feeder.waitUpdate(1000);

  res = onto_ptr->individuals.getUp("alice", 1);
  EXPECT_EQ(res.size(), 4);
  EXPECT_TRUE(std::find(res.begin(), res.end(), "GarfieldOwner") != res.end());     // ownPet value garfield
  EXPECT_TRUE(std::find(res.begin(), res.end(), "PetOwner") != res.end());          // ownPet some (Cat or {rex, pongo})
  EXPECT_TRUE(std::find(res.begin(), res.end(), "PetLover") != res.end());          // ((ownPet min 1) and (ownPet max 3))

  onto_ptr->feeder.removeRelation("alice", "ownPet", "garfield");
  onto_ptr->feeder.waitUpdate(1000);

  res = onto_ptr->individuals.getUp("alice", 1);
  EXPECT_EQ(res.size(), 4);
  EXPECT_TRUE(std::find(res.begin(), res.end(), "NotCatOwner") != res.end());       // ownPet only (not Cat)
  EXPECT_TRUE(std::find(res.begin(), res.end(), "PetOwner") != res.end());          // ownPet some (Cat or {rex, pongo})
  EXPECT_TRUE(std::find(res.begin(), res.end(), "PetLover") != res.end());          // ((ownPet min 1) and (ownPet max 3))

  onto_ptr->feeder.removeRelation("alice", "ownPet", "rex");
  onto_ptr->feeder.waitUpdate(1000);

  res = onto_ptr->individuals.getUp("alice", 1);
  EXPECT_EQ(res.size(), 1);
  EXPECT_TRUE(std::find(res.begin(), res.end(), "Human") != res.end());
}

TEST(reasoning_anonymous_class, no_datatype_same_as)
{
  std::vector<std::string> res;

  // test if no inference exist at initialization
  res = onto_ptr->individuals.getUp("alice", 1);
  EXPECT_EQ(res.size(), 1);
  EXPECT_TRUE(std::find(res.begin(), res.end(), "Human") != res.end());

  onto_ptr->feeder.addRelation("alice", "ownPet", "a");
  onto_ptr->feeder.waitUpdate(1000);

  res = onto_ptr->individuals.getUp("a", 1);
  EXPECT_EQ(res.size(), 1);
  EXPECT_TRUE(std::find(res.begin(), res.end(), "Animal") != res.end());

  res = onto_ptr->individuals.getUp("alice", 1);
  EXPECT_EQ(res.size(), 3);
  EXPECT_TRUE(std::find(res.begin(), res.end(), "NotCatOwner") != res.end());       // ownPet only (not Cat)
  EXPECT_TRUE(std::find(res.begin(), res.end(), "PetLover") != res.end());          // ((ownPet min 1) and (ownPet max 3))

  onto_ptr->feeder.addRelation("a", "=", "garfield");
  onto_ptr->feeder.waitUpdate(1000);

  res = onto_ptr->individuals.getUp("a", 1);
  EXPECT_EQ(res.size(), 2);
  EXPECT_TRUE(std::find(res.begin(), res.end(), "Cat") != res.end());

  res = onto_ptr->individuals.getUp("alice", 1);
  EXPECT_EQ(res.size(), 5);
  EXPECT_TRUE(std::find(res.begin(), res.end(), "GarfieldOwner") != res.end());     // ownPet value garfield
  EXPECT_TRUE(std::find(res.begin(), res.end(), "PetLover") != res.end());          // ((ownPet min 1) and (ownPet max 3))
  EXPECT_TRUE(std::find(res.begin(), res.end(), "ExclusiveCatOwner") != res.end()); // ownPet only Cat
  EXPECT_TRUE(std::find(res.begin(), res.end(), "PetOwner") != res.end());          // ownPet some (Cat or {rex, pongo})

  onto_ptr->feeder.removeRelation("a", "=", "garfield");
  onto_ptr->feeder.waitUpdate(1000);

  res = onto_ptr->individuals.getUp("a", 1);
  EXPECT_EQ(res.size(), 1);
  EXPECT_TRUE(std::find(res.begin(), res.end(), "Animal") != res.end());

  res = onto_ptr->individuals.getUp("alice", 1);
  EXPECT_EQ(res.size(), 3);
  EXPECT_TRUE(std::find(res.begin(), res.end(), "NotCatOwner") != res.end());       // ownPet only (not Cat)
  EXPECT_TRUE(std::find(res.begin(), res.end(), "PetLover") != res.end());          // ((ownPet min 1) and (ownPet max 3))

  onto_ptr->feeder.removeRelation("alice", "ownPet", "a");
  onto_ptr->feeder.waitUpdate(1000);

  res = onto_ptr->individuals.getUp("alice", 1);
  EXPECT_EQ(res.size(), 1);
  EXPECT_TRUE(std::find(res.begin(), res.end(), "Human") != res.end());
}

int main(int argc, char** argv)
{
  ros::init(argc, argv, "ontologenius_reasoning_anonymous_class_test");

  onto::OntologyManipulator onto;
  onto_ptr = &onto;

  onto_ptr->reasoners.activate("ontologenius::ReasonerAnonymous");
  onto.close();
  onto.feeder.waitConnected();

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
