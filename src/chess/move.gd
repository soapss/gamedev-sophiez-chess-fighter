class_name ChessMove

var from: Vector2i
var to: Vector2i
var promotion: ChessPiece.Type = ChessPiece.Type.QUEEN
var is_en_passant: bool = false
var is_castling: bool = false
var is_promotion: bool = false
var captured_piece: ChessPiece = null

func _init(p_from: Vector2i = Vector2i(-1,-1), p_to: Vector2i = Vector2i(-1,-1)):
	from = p_from
	to = p_to

func to_algebraic() -> String:
	var files = "abcdefgh"
	return "%s%d-%s%d" % [files[from.x], 8 - from.y, files[to.x], 8 - to.y]

func _to_string() -> String:
	return to_algebraic()
