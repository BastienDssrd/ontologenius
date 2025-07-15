#include <algorithm>
#include <gtest/gtest.h>
#include <ros/package.h>
#include <ros/ros.h>
#include <string>
#include <vector>

#include "ontologenius/API/ontologenius/OntologiesManipulator.h"

onto::OntologiesManipulator* onto_ptr;

TEST(feature_branching, copy)
{
  std::vector<std::string> res;
  bool res_bool = true;
  std::string test_word = "Robot";

  EXPECT_TRUE(onto_ptr->add("paul"));

  (*onto_ptr)["paul"]->close();

  EXPECT_TRUE(onto_ptr->copy("cpy", "paul"));

  onto::OntologyManipulator* cpy = onto_ptr->get("cpy");
  EXPECT_TRUE(cpy != nullptr);

  cpy->close();

  cpy->feeder.addConcept("table_1");
  cpy->feeder.addConcept("r1");
  cpy->feeder.addConcept("r2");
  cpy->feeder.commit("root");

  cpy->feeder.addRelation("r1", "isOn", "table_1");
  cpy->feeder.commit("b10");

  cpy->feeder.addRelation("r2", "isOn", "r1");
  cpy->feeder.commit("b11");

  cpy->feeder.checkout("root");

  cpy->feeder.addRelation("r2", "isOn", "r1");
  cpy->feeder.commit("b20");

  cpy->feeder.addRelation("r1", "isOn", "table_1");

  EXPECT_FALSE(cpy->feeder.compareCommits("b10", "b20"));
  EXPECT_TRUE(cpy->feeder.compareCommits("b11"));

  cpy->feeder.commit("b21");
  cpy->feeder.checkout("root");

  EXPECT_TRUE(cpy->feeder.compareCommits("b11", "b21"));
}

int main(int argc, char** argv)
{
  ros::init(argc, argv, "ontologenius_feature_branching_test");

  onto::OntologiesManipulator onto;
  onto_ptr = &onto;

  onto.waitInit();

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
