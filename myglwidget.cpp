#define _USE_MATH_DEFINES

#include "myglwidget.h"
#include <GL/glu.h>
#include <QApplication>
#include <QDesktopWidget>

using namespace std;

// Declarations des constantes
const unsigned int WIN_WIDTH  = 1600;
const unsigned int WIN_HEIGHT = 900;
const float MAX_DIMENSION     = 50.0f;


// Constructeur
MyGLWidget::MyGLWidget(QWidget * parent) : QGLWidget(parent)
{
    //antialiasing
    setFormat(QGLFormat(QGL::SampleBuffers));

    //active le suivi de la souris
    setMouseTracking(true);

    // Reglage de la taille/position
    setFixedSize(WIN_WIDTH, WIN_HEIGHT);
    QRect  screenGeometry = QGuiApplication::primaryScreen()->geometry();
    move(screenGeometry.width()/2 - WIN_WIDTH/2, screenGeometry.height()/2 - WIN_HEIGHT/2);

    // Connexion du timer
    connect(&timer,  &QTimer::timeout, [&] {
        timeElapsed += 20;
        updateGL();
    });
    timer.setInterval(20); //tick-rate en ms
    timer.start();
}


// Fonction d'initialisation
void MyGLWidget::initializeGL()
{
    //creation des murs
    MazeGen mazegen ;
    V_walls = mazegen.generateBorder();

    Wall * mur = new Wall( Point(2,0,2), Point(4,0,2) );
    V_walls.push_back(mur);

    Wall * mur2 = new Wall( Point(4,0,2), Point(4,0,4) );
    V_walls.push_back(mur2);

    Wall * mur3 = new Wall( Point(4,0,4), Point(2,0,4) );
    V_walls.push_back(mur3);

    //creation de la sphere
    Sphere * boule = new Sphere(Point(3,1,3), 0.5);
    V_spheres.push_back(boule);

    // creation du plafond et du sol
    Surface * ceiling = new Surface(false);//plafond
    V_surfaces.push_back(ceiling);
    Surface * floor = new Surface(true);//sol
    V_surfaces.push_back(floor);

    // Reglage de la couleur de fond

    // Activation du zbuffer
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
}


// Fonction de redimensionnement
void MyGLWidget::resizeGL(int width, int height)
{
    // Definition du viewport (zone d'affichage)
    glViewport(0, 0, width, height);

    // Definition de la matrice de projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    if(width != 0)
        glOrtho(-MAX_DIMENSION, MAX_DIMENSION, -MAX_DIMENSION * height / static_cast<float>(width), MAX_DIMENSION * height / static_cast<float>(width), -MAX_DIMENSION * 2.0f, MAX_DIMENSION * 2.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}


// Fonction d'affichage
void MyGLWidget::paintGL()
{
    // Reinitialisation des tampons
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //glEnable(GL_LIGHTING);
    // Definition de la matrice projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(FOV, 16.0/9.0, 0.1f, 100.0f);

    // Definition de la matrice modelview
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(player.getPos().x, player.getPos().y, player.getPos().z, //position camera
              player.getTarget().x, player.getTarget().y, player.getTarget().z,  //position cible
              0.0f, 1.0f, 0.0f); //vecteur vertical

    // ---- Affichage sol et plafond ----
    for(Surface * s: V_surfaces){
        s->draw();
    }

    // ---- Affichage des Murs ----
    for(Wall * w: V_walls){
        w->draw();
    }
    // ---- Affichage des Boules ----
    bool AllSpheresFound = true;
    for(Sphere * s: V_spheres){
        s->draw(timeElapsed);
        s->detect(player); // detecte si un joueur ramasse la sphere
        AllSpheresFound = AllSpheresFound & s->isFound(); // vérifie que toutes les spheres sont trouvées
    }
    if (AllSpheresFound && !player.getAchievement()) //evite de répeter la commande si le joueur a déjàtout trouvé
        player.foundSpheres(); // indique au joueur qu'il a trouvé toutes les spheres

    // ---- eclairage ambient ----
    /*
    GLfloat tab_ambient[] = {1.0,1.0,1.0,1.0};
    glLightfv(GL_LIGHT0,GL_AMBIENT, tab_ambient);
    */

    // ---- HUD 2D ----
        //paramétrage
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, WIN_WIDTH, WIN_HEIGHT, 0.0, -1.0, 10.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_CULL_FACE);
    glClear(GL_DEPTH_BUFFER_BIT);
        //carré
    glBegin(GL_QUADS);
        if (player.getAchievement()) {
            glColor3f(0.0f, 1.0f, 0.0);
        } else {
            glColor3f(1.0f, 0.0f, 0.0);
        }
        glVertex2f(0.0, 0.0);
        glVertex2f(100.0, 0.0);
        glVertex2f(100.0, 100.0);
        glVertex2f(0.0, 100.0);
    glEnd();
        //texte
    qglColor(Qt::red);
    renderText(110, 20, QString("Vous jouez depuis %1 secondes").arg(timeElapsed/1000));
    renderText(110, 35, QString("FOV : %1 deg").arg(FOV));
    if (player.getAchievement()){
        renderText(110, 50, QString("Vous avez trouve toutes les spheres, trouvez la sortie !"));
    }

}

// Fonction de gestion d'interactions clavier
void MyGLWidget::keyPressEvent(QKeyEvent * event)
{
   switch(event->key())
    {
        case Qt::Key::Key_Q://gauche
            player.moveWithCollisions(0,-0.1,V_walls);
            break;
        case Qt::Key::Key_D://droite
            player.moveWithCollisions(0,0.1,V_walls);
            break;
        case Qt::Key::Key_Z://avant
            player.moveWithCollisions(0.1,0,V_walls);
            break;
        case Qt::Key::Key_S://arriere
            player.moveWithCollisions(-0.1,0,V_walls);
            break;
        case Qt::Key::Key_E:
            player.look(3,0);
            break;
        case Qt::Key::Key_A:
            player.look(-3,0);
            break;
        case Qt::Key::Key_R:
            mouse ^= true;
            setMouseTracking(mouse);
            break;
        case Qt::Key::Key_F:
            if (fullScreen){
                showNormal();
            } else {
                showFullScreen();
            }
            fullScreen ^= true;
            break;
        case Qt::Key::Key_Tab:
            if (Zbuf){
                glDisable(GL_DEPTH_TEST);
                glDisable(GL_TEXTURE_2D);
            } else { 
                glEnable(GL_DEPTH_TEST);
                glEnable(GL_TEXTURE_2D);
            }
            Zbuf ^= true;
        break;
        case Qt::Key::Key_Escape:
            exit(0);
        break;
        default:
        {
            // Ignorer l'evenement
            event->ignore();
            return;
        }
    }

    // Acceptation de l'evenement et mise a jour de la scene
    event->accept();
    updateGL();
}

void MyGLWidget::wheelEvent(QWheelEvent *event)
{
    float deg = event->angleDelta().y();

    if (deg < 0){
        FOV --;
    } else if ( deg > 0) {
        FOV ++;
    }

    event->accept();
    updateGL();
}

void MyGLWidget::mouseMoveEvent(QMouseEvent *event){
    if (mouse){
        float dx = event->x() - lastPosMouse.x();
        player.look(dx/2.0f,0);
        lastPosMouse = event->pos();
    }
}
