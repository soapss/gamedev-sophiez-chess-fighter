class_name FighterCharacter

enum State { IDLE, ATTACKING, BLOCKING, SPECIAL, HITSTUN }

var piece_type: ChessPiece.Type
var piece_color: ChessPiece.PieceColor
var color_name: String
var hp: int
var max_hp: int
var position_x: float
var state: State = State.IDLE
var state_timer: float = 0.0
var special_cooldown: float = 0.0
var special_max_cooldown: float = 3.0
var block_reduction: float = 0.5 # 50% for normal block, 100% for King shield special

# Move definitions: damage, range, startup, recovery, knockback
var moves: Dictionary = {
	"punch_light": {"damage": 8, "range": 60.0, "startup": 0.1, "recovery": 0.3, "knockback": 10.0},
	"punch_heavy": {"damage": 15, "range": 70.0, "startup": 0.3, "recovery": 0.5, "knockback": 20.0},
	"kick_light": {"damage": 10, "range": 80.0, "startup": 0.15, "recovery": 0.35, "knockback": 12.0},
	"kick_heavy": {"damage": 18, "range": 90.0, "startup": 0.35, "recovery": 0.6, "knockback": 25.0},
	"block": {"damage": 0, "range": 0.0, "startup": 0.05, "recovery": 0.2, "knockback": 0.0},
	"special": {"damage": 15, "range": 70.0, "startup": 0.2, "recovery": 0.5, "knockback": 30.0}
}

func _init(p_type: ChessPiece.Type = ChessPiece.Type.PAWN, p_color: ChessPiece.PieceColor = ChessPiece.PieceColor.WHITE, start_x: float = 200.0):
	piece_type = p_type
	piece_color = p_color
	color_name = "White" if p_color == ChessPiece.PieceColor.WHITE else "Black"
	max_hp = _get_hp_for_type(p_type)
	hp = max_hp
	position_x = start_x
	_setup_special()

func _get_hp_for_type(t: ChessPiece.Type) -> int:
	match t:
		ChessPiece.Type.PAWN: return 80
		ChessPiece.Type.KNIGHT: return 100
		ChessPiece.Type.BISHOP: return 95
		ChessPiece.Type.ROOK: return 120
		ChessPiece.Type.QUEEN: return 130
		ChessPiece.Type.KING: return 110
	return 100

func _setup_special():
	match piece_type:
		ChessPiece.Type.PAWN:
			moves["special"] = {"damage": 15, "range": 70.0, "startup": 0.15, "recovery": 0.4, "knockback": 15.0}
			special_max_cooldown = 3.0
		ChessPiece.Type.KNIGHT:
			moves["special"] = {"damage": 25, "range": 120.0, "startup": 0.25, "recovery": 0.5, "knockback": 35.0}
			special_max_cooldown = 4.5
		ChessPiece.Type.BISHOP:
			moves["special"] = {"damage": 22, "range": 300.0, "startup": 0.3, "recovery": 0.4, "knockback": 10.0}
			special_max_cooldown = 4.0
		ChessPiece.Type.ROOK:
			moves["special"] = {"damage": 30, "range": 150.0, "startup": 0.4, "recovery": 0.6, "knockback": 50.0}
			special_max_cooldown = 5.0
		ChessPiece.Type.QUEEN:
			moves["special"] = {"damage": 35, "range": 500.0, "startup": 0.5, "recovery": 0.7, "knockback": 20.0}
			special_max_cooldown = 6.0
		ChessPiece.Type.KING:
			moves["special"] = {"damage": 20, "range": 80.0, "startup": 0.2, "recovery": 0.5, "knockback": 15.0}
			special_max_cooldown = 5.5

func get_display_name() -> String:
	var type_str = ""
	match piece_type:
		ChessPiece.Type.PAWN: type_str = "Pawn"
		ChessPiece.Type.KNIGHT: type_str = "Knight"
		ChessPiece.Type.BISHOP: type_str = "Bishop"
		ChessPiece.Type.ROOK: type_str = "Rook"
		ChessPiece.Type.QUEEN: type_str = "Queen"
		ChessPiece.Type.KING: type_str = "King"
	return "%s %s" % [color_name, type_str]

func get_special_name() -> String:
	match piece_type:
		ChessPiece.Type.PAWN: return "Pawn Rush"
		ChessPiece.Type.KNIGHT: return "Knight Leap"
		ChessPiece.Type.BISHOP: return "Bishop Warp"
		ChessPiece.Type.ROOK: return "Rook Charge"
		ChessPiece.Type.QUEEN: return "Queen's Wrath"
		ChessPiece.Type.KING: return "Royal Shield"
	return "Special"

func get_hp_percent() -> float:
	if max_hp == 0:
		return 0.0
	return float(hp) / float(max_hp)

func can_attack(target_x: float, move_type: String) -> bool:
	if state == State.HITSTUN or state == State.ATTACKING or state == State.SPECIAL:
		return false
	if move_type == "special" and special_cooldown > 0.0:
		return false
	if not moves.has(move_type):
		return false
	var move_data = moves[move_type]
	var dist = abs(position_x - target_x)
	if move_type != "block" and dist > move_data["range"]:
		return false
	return true

func execute_move(move_type: String):
	if not moves.has(move_type):
		return
	var data = moves[move_type]
	match move_type:
		"block":
			state = State.BLOCKING
			state_timer = 0.4
			block_reduction = 0.5
		"special":
			state = State.SPECIAL
			state_timer = data["startup"] + data["recovery"]
			special_cooldown = special_max_cooldown
			if piece_type == ChessPiece.Type.KING:
				block_reduction = 1.0 # 100% block for Royal Shield duration
			else:
				block_reduction = 0.5
		_:
			state = State.ATTACKING
			state_timer = data["startup"] + data["recovery"]

func take_damage(damage: int, _attacker: FighterCharacter, move_type: String) -> int:
	var actual_damage = damage
	if state == State.BLOCKING:
		actual_damage = int(damage * (1.0 - 0.5))
		print("%s BLOCKED %s: %d -> %d" % [get_display_name(), move_type, damage, actual_damage])
	elif state == State.SPECIAL and piece_type == ChessPiece.Type.KING:
		# Royal Shield blocks all during special
		actual_damage = 0
		print("%s ROYAL SHIELD blocked %s!" % [get_display_name(), move_type])
	hp -= actual_damage
	if hp < 0:
		hp = 0
	if actual_damage > 0:
		state = State.HITSTUN
		state_timer = 0.2 + actual_damage * 0.01
	return actual_damage

func update(delta: float):
	if state_timer > 0:
		state_timer -= delta
		if state_timer <= 0:
			if state == State.SPECIAL and piece_type == ChessPiece.Type.KING:
				block_reduction = 0.5
			state = State.IDLE
			state_timer = 0
	if special_cooldown > 0:
		special_cooldown -= delta
		if special_cooldown < 0:
			special_cooldown = 0

func is_alive() -> bool:
	return hp > 0

func get_special_cooldown_percent() -> float:
	if special_max_cooldown == 0:
		return 0.0
	return special_cooldown / special_max_cooldown
