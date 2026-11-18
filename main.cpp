#include <GL/freeglut.h>
#include <GL/glu.h>
#include <algorithm>  // std::clamp
#include <cmath>      // std::sin, std::cos, std::atan2 e std::hypot

bool usarPerspectiva = true;

struct Transformacao {
    float x = 0, y = 0, z = 0;
    float giroX = 0, giroY = 0, giroZ = 0;
    float escala = 1.0f;
};

Transformacao obras[4];
int selecionada = 0;  // Começa com o cubo selecionado.

const char* titulos[] = {
    "Galeria 3D | 1 - Cubo",
    "Galeria 3D | 2 - Esfera",
    "Galeria 3D | 3 - Cone",
    "Galeria 3D | 4 - Toro"
};

void teclado(unsigned char tecla, int, int) {
    if (tecla >= '1' && tecla <= '4') {
        selecionada = tecla - '1';  // Teclas 1–4 correspondem aos índices 0–3.
        glutSetWindowTitle(titulos[selecionada]);
        return;
    }

    Transformacao& obra = obras[selecionada];
    const float passo = 0.2f;
    const float angulo = 10.0f;

    switch (tecla) {
        case 'a': obra.x -= passo; break;
        case 'd': obra.x += passo; break;
        case 'q': obra.y += passo; break;
        case 'e': obra.y -= passo; break;
        case 'w': obra.z -= passo; break;
        case 's': obra.z += passo; break;

        case 'i': obra.giroX += angulo; break;
        case 'k': obra.giroX -= angulo; break;
        case 'j': obra.giroY += angulo; break;
        case 'l': obra.giroY -= angulo; break;
        case 'u': obra.giroZ += angulo; break;
        case 'o': obra.giroZ -= angulo; break;

        case '+':
        case '=': obra.escala *= 1.1f; break;
        case '-': obra.escala /= 1.1f; break;

        case 'p':
        case 'P':
            usarPerspectiva = !usarPerspectiva;
            break;
        default: return;
    }

    glutPostRedisplay();
}

double proporcaoJanela = 1.0;

void redimensionar(int largura, int altura) {
    largura = std::max(largura, 1);
    altura = std::max(altura, 1);

    glViewport(0, 0, largura, altura);
    proporcaoJanela = static_cast<double>(largura) / altura;
    glutPostRedisplay();
}

void aplicarProjecao() {
    constexpr double campoDeVisao = 45.0;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    if (usarPerspectiva) {
        gluPerspective(campoDeVisao, proporcaoJanela, 0.1, 100.0);
    } else {
        const double meioAngulo = (campoDeVisao / 2.0) * PI / 180.0;
        const double meiaAltura = camera.distancia * std::tan(meioAngulo);
        const double meiaLargura = meiaAltura * proporcaoJanela;

        glOrtho(-meiaLargura, meiaLargura,
                -meiaAltura, meiaAltura,
                0.1, 100.0);
    }

    glMatrixMode(GL_MODELVIEW);
}

// Desenha um bloco a partir de um cubo de lado 1.
void desenharBloco(float x, float y, float z, float largura, float altura, float profundidade) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(largura, altura, profundidade);
    glutSolidCube(1.0);
    glPopMatrix();
}

void desenharSala() {
    // Piso: a superfície superior fica em y = 0.
    glColor3f(0.30f, 0.32f, 0.36f);
    desenharBloco(0, -0.1f, 0, 12, 0.2f, 8);

    // Parede do fundo.
    glColor3f(0.20f, 0.23f, 0.29f);
    desenharBloco(0, 2, -4, 12, 4, 0.15f);

    // Paredes laterais.
    glColor3f(0.25f, 0.28f, 0.34f);
    desenharBloco(-6, 2, 0, 0.15f, 4, 8);
    desenharBloco( 6, 2, 0, 0.15f, 4, 8);
}

constexpr double PI = 3.14159265358979323846;

struct Camera {
    double alvoX = 0.0;
    double alvoY = 1.0;
    double alvoZ = 0.0;

    // Valores que reproduzem a posição inicial (0, 6, 13) da T03.
    double distancia = std::hypot(5.0, 13.0);
    double horizontal = 0.0;
    double vertical = std::atan2(5.0, 13.0);
};

Camera camera;

void aplicarCamera() {
    const double raioHorizontal = camera.distancia * std::cos(camera.vertical);

    const double x = camera.alvoX + raioHorizontal * std::sin(camera.horizontal);
    const double y = camera.alvoY + camera.distancia * std::sin(camera.vertical);
    const double z = camera.alvoZ + raioHorizontal * std::cos(camera.horizontal);

    gluLookAt(x, y, z,
              camera.alvoX, camera.alvoY, camera.alvoZ,
              0.0, 1.0, 0.0);
}

void tecladoEspecial(int tecla, int, int) {
    const double passoGiro = 5.0 * PI / 180.0;
    const double limiteVertical = 85.0 * PI / 180.0;
    const double passoMovimento = 0.4;
    const bool shift = (glutGetModifiers() & GLUT_ACTIVE_SHIFT) != 0;

    if (shift) {
        // Direções relativas à tela, inclusive depois de girar a câmera.
        double direita = 0.0;
        double cima = 0.0;

        switch (tecla) {
            case GLUT_KEY_LEFT:  direita = -passoMovimento; break;
            case GLUT_KEY_RIGHT: direita =  passoMovimento; break;
            case GLUT_KEY_UP:    cima =  passoMovimento; break;
            case GLUT_KEY_DOWN:  cima = -passoMovimento; break;
            default: return;
        }

        const double h = camera.horizontal;
        const double v = camera.vertical;

        camera.alvoX += direita * std::cos(h) - cima * std::sin(v) * std::sin(h);
        camera.alvoY += cima * std::cos(v);
        camera.alvoZ -= direita * std::sin(h) + cima * std::sin(v) * std::cos(h);
    } else {
        switch (tecla) {
            case GLUT_KEY_LEFT:      camera.horizontal -= passoGiro; break;
            case GLUT_KEY_RIGHT:     camera.horizontal += passoGiro; break;
            case GLUT_KEY_UP:        camera.vertical += passoGiro; break;
            case GLUT_KEY_DOWN:      camera.vertical -= passoGiro; break;
            case GLUT_KEY_PAGE_UP:   camera.distancia -= 0.6; break;
            case GLUT_KEY_PAGE_DOWN: camera.distancia += 0.6; break;
            default: return;
        }

        camera.vertical = std::clamp(camera.vertical, -limiteVertical, limiteVertical);
        camera.distancia = std::clamp(camera.distancia, 2.0, 35.0);
    }

    glutPostRedisplay();
}

void desenhar() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    aplicarProjecao();
    glLoadIdentity();
    aplicarCamera();

    desenharSala();


    // As obras da frente ficam mais afastadas entre si.
    const float posicoes[4][2] = {
        {-3.6f,  1.7f},  // Cubo
        { 3.6f,  1.7f},  // Esfera
        {-1.7f, -1.8f},  // Cone
        { 1.7f, -1.8f}   // Toro
    };

    for (int i = 0; i < 4; ++i) {
        const float x = posicoes[i][0];
        const float z = posicoes[i][1];

        // O pedestal vai de y = 0 até y = 0.8.
        glColor3f(0.62f, 0.64f, 0.68f);
        desenharBloco(x, 0.4f, z, 2, 0.8f, 2);

        glPushMatrix();

        const Transformacao& obra = obras[i];
            
        // Distância entre o topo do pedestal e o centro de cada forma.
        const float alturasCentro[] = {0.6f, 0.75f, 0.8f, 0.85f};
        const float centro = alturasCentro[i];
            
        glTranslatef(x + obra.x, 0.8f + centro + obra.y, z + obra.z);
            
        glRotatef(obra.giroX, 1, 0, 0);
        glRotatef(obra.giroY, 0, 1, 0);
        glRotatef(obra.giroZ, 0, 0, 1);
        glScalef(obra.escala, obra.escala, obra.escala);
            
        glTranslatef(0, -centro, 0);

        switch (i) {
            case 0:
                glColor3f(0.20f, 0.85f, 1.00f);
                glTranslatef(0, 0.6f, 0);
                glRotatef(25, 0, 1, 0);
                glutWireCube(1.2);
                break;

            case 1:
                glColor3f(1.00f, 0.75f, 0.20f);
                glTranslatef(0, 0.75f, 0);
                glutWireSphere(0.75, 18, 12);
                break;

            case 2:
                glColor3f(0.35f, 1.00f, 0.55f);
                glRotatef(-90, 1, 0, 0);  // Aponta o cone para cima.
                glutWireCone(0.75, 1.6, 16, 5);
                break;

            case 3:
                glColor3f(1.00f, 0.45f, 0.65f);
                glTranslatef(0, 0.85f, 0);
                glutWireTorus(0.2, 0.65, 10, 24);
                break;
        }

        glPopMatrix();
    }

    glutSwapBuffers();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Galeria 3D - Teste inicial");

    glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    glutReshapeFunc(redimensionar);
    glutDisplayFunc(desenhar);

    glutKeyboardFunc(teclado);
    glutSetWindowTitle(titulos[selecionada]);

    glutKeyboardFunc(teclado);          // T04: números e letras controlam as obras.
    glutSpecialFunc(tecladoEspecial);   // T05: setas e Page Up/Down controlam a câmera.

    glutMainLoop();
    return 0;
}

