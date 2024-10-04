#include"HelloBox2d.h"

//box2dチュートリアル
//=============================================
//https://box2d.org/documentation/hello.html
//=============================================

b2WorldId worldId;
b2BodyId bodyId;
b2DebugDraw debugDraw;

void HelloBox2dInit()
{
	//================================================================
	// Creating a Ground Box
	//================================================================
	
	//ワールド定義、初期化
	b2WorldDef worldDef = b2DefaultWorldDef();

	//重力の設定
	worldDef.gravity = { 0.0f, -10.0f };

	//ワールドオブジェクト作成
	worldId = b2CreateWorld(&worldDef);

	//ボディ定義とワールドIDを使ってグラウンド・ボディを作成する
	b2BodyDef groundBodyDef = b2DefaultBodyDef();
	groundBodyDef.position = { 0.0f, -10.0f };

	//グランドボディの作成
	b2BodyId groundId = b2CreateBody(worldId, &groundBodyDef);

	//地面ポリゴンを作る。 b2MakeBox()ヘルパー関数を使い、地面ポリゴンを箱型にする。箱の中心は親ボディの原点である。
	b2Polygon groundBox = b2MakeBox(50.0f, 10.0f);

	//シェイプを作成して地面のボディを仕上げる
	b2ShapeDef groundShapeDef = b2DefaultShapeDef();
	b2CreatePolygonShape(groundId, &groundShapeDef, &groundBox);


	//================================================================
	// Creating a Dynamic Body
	//================================================================

	//CreateBodyを使ってボディを作成する。 デフォルトではボディは静的なので、作成時にb2BodyTypeを設定してボディを動的にする
	//ボディを作成してから移動させるのは非常に非効率的のため作成する前に動かす
	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = b2_dynamicBody;
	bodyDef.position = { 0.0f, 4.0f };
	bodyId = b2CreateBody(worldId, &bodyDef);

	//※注意 ボディを力（重力など）に反応して動かしたい場合は、ボディタイプをb2_dynamicBodyに設定する必要があります。

	//シェイプ定義を使ってポリゴン形状を作成し、アタッチする。 まず、別のボックスシェイプを作成する
	b2Polygon dynamicBox = b2MakeBox(1.0f, 1.0f);

	//ボックスの形状定義を作成する。 デフォルトの密度は1なので、これは不要である。 また、形状の摩擦は0.3に設定されている。
	b2ShapeDef shapeDef = b2DefaultShapeDef();
	shapeDef.density = 1.0f;
	shapeDef.friction = 0.3f;

	//※注意 ダイナミックボディは、密度がゼロでない形状を少なくとも1つ持つべきである。 そうでないと、奇妙な動作になる

	//シェイプの定義を使って、シェイプを作ることができる。 これでボディの質量が自動的に更新される。 
	//ボディには好きなだけ形状を追加できます。 それぞれが総質量に寄与する。
	b2CreatePolygonShape(bodyId, &shapeDef, &dynamicBox);
}

void HelloBox2dUpdate()
{
	//================================================================
	// Simulating the World
	//================================================================

	//Box2Dのタイム・ステップを選ぶ
	//また、タイムステップがフレームごとに異なるのもよくありません。 タイムステップが変わると結果が変わり、デバッグが難しくなります。
	//  だから、タイムステップをフレームレートに結びつけてはいけない
	float timeStep = 1.0f / 60.0f;

	//Box2Dは、制約の反復の手段としてサブステップを使用します。 Box2Dの推奨サブステップ数は4です。
	int subStepCount = 4;

	//タイムステップとサブステップ数は関連していることに注意。 タイムステップが小さくなると、サブステップのサイズも小さくなる。 
	//例えば、60Hzのタイムステップと4つのサブステップでは、サブステップは240Hzで動作します
	//ゲームでは、シミュレーションループをゲームループにマージすることができます。 ゲームループを通過するたびに、b2World_Step()を呼び出す。 
	//フレームレートや物理演算のタイムステップにもよるが、通常は1回呼び出すだけで十分だ。
	for (int i = 0; i < 90; ++i)
	{
		b2World_Step(worldId, timeStep, subStepCount);
		b2Vec2 position = b2Body_GetPosition(bodyId);
		b2Rot rotation = b2Body_GetRotation(bodyId);
		printf("%4.2f %4.2f %4.2f\n", position.x, position.y, b2Rot_GetAngle(rotation));
	}
}

void HelloBox2dDraw()
{
	debugDraw.drawShapes = true;
	b2World_Draw(worldId, &debugDraw);
}

void HelloBox2dUninit()
{
	//シミュレーションが終わったら、世界を破壊しなければならない。
	b2DestroyWorld(worldId);
}


