workflow "build" {
  on = "push"
  resolves = ["build_package"]
}

action "build_package" {
  uses = "./docker/package"
}
