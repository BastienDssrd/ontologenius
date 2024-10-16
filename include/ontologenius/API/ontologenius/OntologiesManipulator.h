#ifndef ONTOLOGENIUS_ONTOLOGIESMANIPULATOR_H
#define ONTOLOGENIUS_ONTOLOGIESMANIPULATOR_H

#include <vector>
#include <string>

#include <ros/ros.h>

#include "ontologenius/API/ontologenius/clients/ManagerClient.h"
#include "ontologenius/API/ontologenius/OntologyManipulator.h"

/// @brief The OntologiesManipulator class allows to create and delete ontologies instances dynamically.
/// Its usage is strongly recommended for multi-ontology usage and ensures good thread management.
/// This class allows you to set up the theory of mind quickly in a software application.
class OntologiesManipulator : public ManagerClient
{
public:
  /// @brief Constructs a manipulator for several instance of ontologies with a pointer to a NodeHandle n.
  /// This pointer is necessary to create the ros services in the constructor.
  /// @param n is an initialized ros node handle
  explicit OntologiesManipulator(ros::NodeHandle* n);
  ~OntologiesManipulator();

  /// @brief Wait for ontologenius services to be advertised and available for. Blocks until it is.
  /// @param timeout is the amount of time to wait for before timing out. If timeout is -1 (default), waits until the node is shutdown.
  void waitInit(int32_t timeout = -1);

  /// @brief Gets a pointer on the OntologyManipulator instance named name.
  /// @param name is the name of the instance to get.
  /// @return nullptr if no OntologyManipulator instance is named name.
  OntologyManipulator* operator[](const std::string& name);
  /// @brief Gets a pointer on the OntologyManipulator instance named name.
  /// @param name is the name of the instance to get.
  /// @return Returns nullptr if no OntologyManipulator instance is named name.
  OntologyManipulator* get(const std::string& name);

  /// @brief Creates a new instance of ontologyManipulator identified by the name name.
  /// @param name is the name of the instance to create
  /// @return Returns false if the creation fails. Returns true even if the instance already exists.
  bool add(const std::string& name);
  /// @brief Creates a new instance of ontologyManipulator identified by the name dest_name that manipulates a copy of the ontology handled by the ontologyManipulator src_name.
  /// @param dest_name is the new of the instance to create.
  /// @param src_name is the new of the instance to copy.
  /// @return Returns false if the copy fails. Returns true even if the instance already exists.
  bool copy(const std::string& dest_name, const std::string& src_name);
  /// @brief Deletes the instance of ontologyManipulator identified by the name name.
  /// @param name is the name of the instance to delete.
  /// @return Returns false deletion fails. Returns true even if the instance does not exist.
  bool del(const std::string& name);

  /// @brief If verbose is set to true, the clients will post messages about the failure to call the services and about their restoration.
  void verbose(bool verbose) { ClientBase::verbose(verbose); }

private:
  ros::NodeHandle* n_; // do not move this line below
  std::map<std::string, OntologyManipulator*> manipulators_;
};

#endif // ONTOLOGENIUS_ONTOLOGIESMANIPULATOR_H
