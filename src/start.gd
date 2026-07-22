extends Control
# Starting screen for Chess Fighter one-shot
# Shows CHESS FIGHTER in blocky letters + PLAY button in lower third
# On PLAY -> go to chess board (Main.tscn)

func _ready():
	print("Start screen ready - Play button should be in lower third")
	# Ensure button is focusable and on top
	var btn = $CenterContainer/VBox/PlayButton
	if btn:
		btn.grab_focus()
		print("PlayButton found: ", btn.get_path())

func _on_play_pressed():
	print("PLAY pressed -> switching to chess board")
	var err = get_tree().change_scene_to_file("res://src/scenes/Main.tscn")
	if err != OK:
		print("ERROR switching scene: ", err, " - check Main.tscn path and validity")
		# Fallback: try loading and instantiating
		var scene = load("res://src/scenes/Main.tscn")
		if scene:
			print("Main.tscn loaded successfully, trying deferred change")
			get_tree().call_deferred("change_scene_to_file", "res://src/scenes/Main.tscn")
		else:
			print("FAILED to load Main.tscn - file missing or parse error")
	else:
		print("Scene switch initiated OK")

func _on_play_button_down():
	print("PlayButton down event")
