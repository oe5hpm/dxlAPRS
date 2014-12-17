<*+ MAIN *>
MODULE runme;

(* Top-level module *)

IMPORT Nodes, Echo, GenEcho, Commands, Types;

BEGIN
  Echo.Insert;
  GenEcho.Insert;
  Commands.Insert;
  Types.Insert;
  Nodes.Help;
  Nodes.Loop;
END runme.
