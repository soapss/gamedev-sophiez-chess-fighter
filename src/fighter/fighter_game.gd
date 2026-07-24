class_name FighterGame

signal fighter_hit(attacker: FighterCharacter, defender: FighterCharacter, damage: int, move_type: String)
signal fighter_ko(winner: FighterCharacter, loser: FighterCharacter)
signal fighter_special_used(user: FighterCharacter, name: String)

var fighter1: FighterCharacter # attacker left
var fighter2: FighterCharacter # defender right
var arena_width: float = 800.0
var round_time: float = 45.0
var time_remaining: float = 45.0
var is_finished: bool = false

func _init(attacker_piece: ChessPiece, defender_piece: ChessPiece):
	# attacker starts at 200, defender at 600
	fighter1 = FighterCharacter.new(attacker_piece.type, attacker_piece.color, 200.0)
	fighter2 = FighterCharacter.new(defender_piece.type, defender_piece.color, 600.0)
	time_remaining = round_time
	is_finished = false
	print("FighterGame created: %s (%d HP %s) vs %s (%d HP %s)" % [
		fighter1.get_display_name(), fighter1.hp, fighter1.get_special_name(),
		fighter2.get_display_name(), fighter2.hp, fighter2.get_special_name()
	])

# Returns true if finished
func update(delta: float) -> bool:
	if is_finished:
		return true

	time_remaining -= delta
	fighter1.update(delta)
	fighter2.update(delta)

	# Check KO
	if not fighter1.is_alive() or not fighter2.is_alive():
		is_finished = true
		var winner = fighter1 if fighter1.is_alive() else fighter2
		var loser = fighter2 if fighter1.is_alive() else fighter1
		fighter_ko.emit(winner, loser)
		print("Fighter KO: %s wins" % winner.get_display_name())
		return true

	# Timeout - higher HP wins, tie attacker wins per spec
	if time_remaining <= 0:
		is_finished = true
		time_remaining = 0
		print("Fighter timeout - HP %d vs %d" % [fighter1.hp, fighter2.hp])
		# attacker_won logic handles tie
		return true

	return false

func get_time_remaining() -> float:
	return time_remaining

func attacker_won() -> bool:
	if not fighter1.is_alive():
		return false
	if not fighter2.is_alive():
		return true
	# timeout case
	if fighter1.hp > fighter2.hp:
		return true
	elif fighter2.hp > fighter1.hp:
		return false
	else:
		# tie per spec attacker wins
		return true

func try_fighter1_move(move_type: String) -> bool:
	if is_finished:
		return false
	if not fighter1.can_attack(fighter2.position_x, move_type):
		# For movement, we handle outside, for attacks check range
		if move_type == "block" or move_type == "special":
			# special can be used even if out of range for some types (Queen fireball etc) - we allow if cooldown ok
			if move_type == "special" and fighter1.special_cooldown > 0:
				return false
		else:
			# Check if out of range, fail silently
			pass

	# Special handling for movement done in main.gd via position_x directly
	# For attacks, execute and check hit
	if not fighter1.moves.has(move_type):
		return false

	fighter1.execute_move(move_type)

	if move_type == "special":
		fighter_special_used.emit(fighter1, fighter1.get_special_name())
		# Bishop Warp: teleport behind
		if fighter1.piece_type == ChessPiece.Type.BISHOP:
			fighter1.position_x = fighter2.position_x + (20 if fighter1.position_x < fighter2.position_x else -20)
			print("Bishop Warp! New pos %d" % int(fighter1.position_x))
		# Knight Leap: jump forward evading low
		if fighter1.piece_type == ChessPiece.Type.KNIGHT:
			fighter1.position_x += 80 if fighter1.position_x < fighter2.position_x else -80
			fighter1.position_x = clamp(fighter1.position_x, 50.0, arena_width - 50.0)

	# Check if hits
	if move_type != "block":
		var dist = abs(fighter1.position_x - fighter2.position_x)
		var move_data = fighter1.moves[move_type]
		if dist <= move_data["range"]:
			var dmg = move_data["damage"]
			var actual = fighter2.take_damage(dmg, fighter1, move_type)
			fighter_hit.emit(fighter1, fighter2, actual, move_type)
			# Apply knockback
			var dir = 1 if fighter2.position_x > fighter1.position_x else -1
			fighter2.position_x += move_data["knockback"] * dir
			fighter2.position_x = clamp(fighter2.position_x, 50.0, arena_width - 50.0)
			print("HIT %s -> %s %d dmg (%s) dist %.1f" % [fighter1.get_display_name(), fighter2.get_display_name(), actual, move_type, dist])
			return true
		else:
			print("MISS %s %s dist %.1f > range %.1f" % [fighter1.get_display_name(), move_type, dist, move_data["range"]])
			return false
	return true

func try_fighter2_ai_move() -> bool:
	if is_finished:
		return false
	# Simple AI per previous gold: low HP block, special chance, close random, far move closer
	if not fighter2.is_alive():
		return false

	# If in hitstun or attacking, wait
	if fighter2.state != FighterCharacter.State.IDLE and fighter2.state != FighterCharacter.State.BLOCKING:
		return false

	var dist = abs(fighter1.position_x - fighter2.position_x)

	# Low HP -> block
	if fighter2.get_hp_percent() < 0.3 and randf() < 0.4:
		return try_fighter2_exec("block")

	# Special chance if ready and somewhat close or queen/bishop long range
	if fighter2.special_cooldown <= 0 and randf() < 0.25:
		if dist <= fighter2.moves["special"]["range"] or fighter2.piece_type == ChessPiece.Type.QUEEN or fighter2.piece_type == ChessPiece.Type.BISHOP:
			return try_fighter2_exec("special")

	# If far, move closer
	if dist > 100:
		var dir = -1 if fighter2.position_x > fighter1.position_x else 1
		fighter2.position_x += dir * 15.0 * randf_range(0.5, 1.5)
		fighter2.position_x = clamp(fighter2.position_x, 50.0, arena_width - 50.0)
		return false

	# Close -> random attack
	var choices = ["punch_light", "punch_heavy", "kick_light", "kick_heavy"]
	var pick = choices[randi() % choices.size()]
	if randf() < 0.6:
		return try_fighter2_exec(pick)
	return false

func try_fighter2_exec(move_type: String) -> bool:
	if not fighter2.can_attack(fighter1.position_x, move_type) and move_type != "block":
		# For special long range still allow
		if not (move_type == "special" and fighter2.moves["special"]["range"] >= abs(fighter1.position_x - fighter2.position_x)):
			return false

	fighter2.execute_move(move_type)
	if move_type == "special":
		fighter_special_used.emit(fighter2, fighter2.get_special_name())
		if fighter2.piece_type == ChessPiece.Type.BISHOP:
			fighter2.position_x = fighter1.position_x + (20 if fighter2.position_x < fighter1.position_x else -20)
		if fighter2.piece_type == ChessPiece.Type.KNIGHT:
			fighter2.position_x += 80 if fighter2.position_x < fighter1.position_x else -80
			fighter2.position_x = clamp(fighter2.position_x, 50.0, arena_width - 50.0)

	if move_type != "block":
		var dist = abs(fighter2.position_x - fighter1.position_x)
		var move_data = fighter2.moves[move_type]
		if dist <= move_data["range"]:
			var dmg = move_data["damage"]
			var actual = fighter1.take_damage(dmg, fighter2, move_type)
			fighter_hit.emit(fighter2, fighter1, actual, move_type)
			var dir = 1 if fighter1.position_x > fighter2.position_x else -1
			fighter1.position_x += move_data["knockback"] * dir
			fighter1.position_x = clamp(fighter1.position_x, 50.0, arena_width - 50.0)
			return true
	return false
