workflow "Build" {
  on = "push"
  resolves = ["build_action"]
}

action "build_action" {
  uses = "./docker/package"
}
