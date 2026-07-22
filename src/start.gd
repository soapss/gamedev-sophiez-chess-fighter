extends Control
# Starting screen for Chess Fighter one-shot
# Shows CHESS FIGHTER in blocky letters + PLAY button in lower third
# On PLAY -> go to chess board (Main.tscn)

func _ready():
	# Optionally set background or animate title
	pass

func _on_play_pressed():
	print("PLAY pressed -> switching to chess board")
	get_tree().change_scene_to_file("res://src/scenes/Main.tscn")
